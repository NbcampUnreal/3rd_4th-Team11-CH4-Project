#include "Survivor/Character/EHSurvivorCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "gimmick/Door/MultiplayDoor.h"
#include "gimmick/Generators/Generators.h"
#include "gimmick/Keypad/MultiKeypad.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "Blueprint/UserWidget.h"
#include "Item/Chest/Chest.h"
#include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "GameMode/Prison/PrisonManager.h"
#include "Item/Inventory/InventoryComponent.h"
#include "UI/Widgets/InGameHUD/Status/InteractProgressWidget.h"
#include "Net/UnrealNetwork.h"

AEHSurvivorCharacter::AEHSurvivorCharacter()
	: bCanAttack(true)
	, AttackMontagePlayTime(0.f)
	, DeathMontagePlayTime(0.f)
	, bIsCrouching(false)
	, bIsInteracting(false)
	, bIsVaulting(false)
	, InterActionLength(300.0f)
	, InteractActor(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;
	// 레플리케이션 설정
	bReplicates = true;
	
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetIsReplicated(true);
	}

	if (GetMesh())
	{
		GetMesh()->SetIsReplicated(true);
	}
	
	// 속성 컴포넌트 생성
	AttributeComp = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComp"));

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetCapsuleComponent());
	SpringArm->TargetArmLength = 400.f;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArm);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 450.0f, 0.0f);
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	AttributeComp->SetCurrentSpeed(AttributeComp->BaseSpeed);

	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(2);
}

void AEHSurvivorCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 상호작용 거리 오프셋 조절
	InterActionLength += SpringArm->TargetArmLength;

	if (SpringArm)
	{
		DefaultSpringArmLocation = SpringArm->GetRelativeLocation();
		CrouchSpringArmLocation = DefaultSpringArmLocation + FVector(0, 0, 48.5);
	}

	if (IsValid(AttackMontage))
	{
		AttackMontagePlayTime = AttackMontage->GetPlayLength();
	}
	if (IsValid(DeathMontage))
	{
		DeathMontagePlayTime = DeathMontage->GetPlayLength();
	}

	// 🔹 클라이언트(혹은 리슨 서버의 로컬플레이어)에서만 입력 매핑
	if (GetNetMode() != NM_DedicatedServer && IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (ULocalPlayer* LP = PC->GetLocalPlayer())
			{
				if (UEnhancedInputLocalPlayerSubsystem* EILPS =
						LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
				{
					EILPS->AddMappingContext(InputMappingContext, 0);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("EnhancedInputLocalPlayerSubsystem is NULL (client only)."));
				}
			}
		}
	}

	// ✅ PlayerState 상태 변경 반응 등록
	if (AInGamePlayerState* PS = GetPlayerState<AInGamePlayerState>())
	{
		HandleStateChange(PS->SurvivorState);
	}

	//상호작용 UI 위젯 생성
	if (IsLocallyControlled() && InteractProgressClass)
	{
		InteractProgressWidget = Cast<UInteractProgressWidget>(CreateWidget<UUserWidget>(GetWorld(), InteractProgressClass));
	}
}

void AEHSurvivorCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bCanAttack);
	DOREPLIFETIME(ThisClass, bIsDead);
	DOREPLIFETIME(ThisClass, bIsInteracting);
	DOREPLIFETIME(ThisClass, InteractActor);
}

//============================== 사운드 ==============================
void AEHSurvivorCharacter::PlayFootstepSound()
{
	if (FootstepSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			FootstepSound,
			GetActorLocation(),
			1.0f,
			1.0f,
			0.0f,
			FootstepAttenuation
		);
	}
}

//======================================================================

//============================== 피격, 죽음 ==============================
void AEHSurvivorCharacter::MulticastHitMontage_Implementation()
{
	//피격시 일정시간 속도 증가 및 맵에 표시
	AttributeComp->ApplySpeedBuff(500, 3.f);
	GetMesh()->SetCustomDepthStencilValue(3);
	GetWorldTimerManager().SetTimer(DeleteHitMarkHandle,this,&AEHSurvivorCharacter::DeleteHitMark, 3.f, false);
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		// 이미 재생 중이면 중단 후 다시 재생
		AnimInstance->Montage_Stop(0.1f, HitReactMontage);
		AnimInstance->Montage_Play(HitReactMontage);
	}
}

void AEHSurvivorCharacter::MulticastDeathMontage_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AEHSurvivorCharacter::OnDeathMontageEnded);
		AnimInstance->OnMontageEnded.AddDynamic(this, &AEHSurvivorCharacter::OnDeathMontageEnded);
		AnimInstance->Montage_Play(DeathMontage);
	}
}
void AEHSurvivorCharacter::OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bIsDead)
	{
		GetMesh()->SetVisibility(false, true);
		// 죽는 모션 먼저 재생 후 처리 코드
		Destroy();
	}
}
//======================================================================

// ============================== 데미지 처리 (기존 유지 + 감옥 반응) ==============================
float AEHSurvivorCharacter::TakeDamage(
	float DamageAmount,
	const FDamageEvent& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	if (!HasAuthority())
		return 0.f;

	const float OldHP = AttributeComp->GetHealth();
	AttributeComp->TakeDamage(DamageAmount);
	const float NewHP = AttributeComp->GetHealth();
	
	AInGamePlayerState* PS = GetPlayerState<AInGamePlayerState>();
	if (!PS) return 0.f;
	
	if (NewHP <= 0.f && !bIsDead)
	{
		PS->ServerSetSurvivorState(ESurvivorState::Captured); // 감옥 수감 처리
		MulticastDeathMontage();
	}
	else if (NewHP < OldHP)
	{
		PS->ServerSetSurvivorState(ESurvivorState::Injured);
		MulticastHitMontage();
	}

	return OldHP - NewHP;
}

// ============================== 🔹 감옥 시스템 추가 ==============================
void AEHSurvivorCharacter::HandleStateChange(ESurvivorState NewState)
{
	switch (NewState)
	{
	case ESurvivorState::Captured:
		{
			if (HasAuthority())
			{
				for (TActorIterator<APrisonManager> It(GetWorld()); It; ++It)
				{
					if (APrisonManager* Prison = *It)
					{
						Prison->Capture(this);
						break;
					}
				}
			}
			else
			{
				ServerRequestCapture(); // 클라면 서버로 RPC 요청
			}
			break;
		}
	case ESurvivorState::Alive:
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			EnableInput(nullptr);
			bIsDead = false;
			break;
		}
	default:
		break;
	}
}

void AEHSurvivorCharacter::ServerRequestCapture_Implementation()
{
	for (TActorIterator<APrisonManager> It(GetWorld()); It; ++It)
	{
		if (APrisonManager* Prison = *It)
		{
			Prison->Capture(this);
			break;
		}
	}
}

//======================================================================

void AEHSurvivorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}

//============================== 인풋 바인딩 ==============================
void AEHSurvivorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Item : 임시 디버그용
	PlayerInputComponent->BindKey(EKeys::T, IE_Pressed, this, &AEHSurvivorCharacter::Debug_TakeDamage);
	PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &AEHSurvivorCharacter::UseSlot0);
	PlayerInputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AEHSurvivorCharacter::UseSlot1);

	UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
	EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
	EIC->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::ToggleCrouch);
	EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &ThisClass::AttackInput);
	EIC->BindAction(InteractAction,ETriggerEvent::Started,this,&AEHSurvivorCharacter::StartInteraction);
	EIC->BindAction(InteractAction, ETriggerEvent::Completed, this, &AEHSurvivorCharacter::ServerRPCCancelInteract);
	EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &AEHSurvivorCharacter::Jump);
}
//======================================================================

//============================== 이동, 시점 ==============================
void AEHSurvivorCharacter::Move(const FInputActionValue& value)
{
	if (IsValid(Controller) == false)
	{
		return;
	}
	
	if (bIsVaulting) return;
	
	const FVector2D InMovementVector = value.Get<FVector2D>();

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator ControlYawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, InMovementVector.X);
	AddMovementInput(RightDirection, InMovementVector.Y);

	if (bIsInteracting) ServerRPCCancelInteract();
}

void AEHSurvivorCharacter::Look(const FInputActionValue& value)
{
	if (IsValid(Controller) == false)
	{
		return;
	}
	
	const FVector2D InLookVector = value.Get<FVector2D>();

	AddControllerYawInput(InLookVector.X);
	AddControllerPitchInput(InLookVector.Y);
}
//======================================================================

//============================== 앉기 ==============================
void AEHSurvivorCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	bIsCrouching = true;
	
	if (SpringArm)
	{
		SpringArm->SetRelativeLocation(CrouchSpringArmLocation);
	}

	if (bIsInteracting) ServerRPCCancelInteract();
}
void AEHSurvivorCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	bIsCrouching = false;
	
	if (SpringArm)
	{
		SpringArm->SetRelativeLocation(DefaultSpringArmLocation);
	}

	if (bIsInteracting) ServerRPCCancelInteract();
}
void AEHSurvivorCharacter::ToggleCrouch(const FInputActionValue& value)
{
	if (bIsCrouching)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

//======================================================================

//============================== 상호작용 ==============================
void AEHSurvivorCharacter::StartInteraction()
{
	//bIsInteracting = true;
	if (!HasAuthority())
	{
		ServerRPCInteract();
		return;
	}
	
	const FVector LineTraceStart = CameraComp->GetComponentLocation();
	const FVector LineTraceEnd = LineTraceStart + FVector(CameraComp->GetForwardVector()*InterActionLength);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult,LineTraceStart,LineTraceEnd,ECC_Visibility,Params))
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			UE_LOG(LogTemp,Log,TEXT("%s"),*HitActor->GetName());
			if (AMultiplayDoor* Door = Cast<AMultiplayDoor>(HitActor))
			{
				if (Door->GetIsOpenDoor()) return;
				if (Door->GetIsRemoteDoor()) return;
				//아이템(키) 소지 여부 확인 및 사용 로직
				if (UInventoryComponent* Inv = FindComponentByClass<UInventoryComponent>())
				{
					if (Inv->ConsumeKey())   // 키가 있으면 true + 소모됨
					{
						MulticastDoorInteractMontage();
						ServerRTryOpenDoorWithKey(Door); // 서버에서 실제 개방
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("열쇠가 없어 문을 열 수 없습니다."));
					}
				}
				return;
				
				// ServerRPCInteractionDoorOnPawn(Door);
			}
			if (AGenerators* Generator = Cast<AGenerators>(HitActor))
			{
				if (!bIsInteracting)
				{
					GetCharacterMovement()->DisableMovement();
					bIsInteracting = true;
					InteractActor = Generator;
					MulticastInteractMontage();
					ServerRPCStartInteract();
				}
			}
			if (AMultiKeypad* Keypad = Cast<AMultiKeypad>(HitActor))
			{
				if (!bIsInteracting)
				{
					bIsInteracting = true;
					InteractActor = Keypad;
					MulticastInteractMontage();
					ServerRPCStartInteract();
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp,Log,TEXT("LineTraceMiss"));
	}
}

void AEHSurvivorCharacter::Jump()
{
	Super::Jump();
}

void AEHSurvivorCharacter::ServerRPCInteractionDoorOnPawn_Implementation(class AMultiplayDoor* Door)
{
	Door->InterActionDoor();
}

void AEHSurvivorCharacter::MulticastInteractMontage_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) == true)
	{
		AnimInstance->StopAllMontages(0.f);
		AnimInstance->Montage_Play(InteractMontage);
	}
}

void AEHSurvivorCharacter::MulticastStopInteractMontage_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) && InteractMontage)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		AnimInstance->Montage_Stop(0.1f, InteractMontage);
	}
}

void AEHSurvivorCharacter::MulticastDoorInteractMontage_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) == true)
	{
		AnimInstance->StopAllMontages(0.f);
		AnimInstance->Montage_Play(DoorInteractMontage, 5.f);
	}
}
//======================================================================

//============================== 발전기, 키패드 상호작용 ==============================
void AEHSurvivorCharacter::ServerRPCStartInteract_Implementation()
{
	if (AGenerators* Generator = Cast<AGenerators>(InteractActor))
	{
		Generator->StartRepair(this);
	}
	if (AMultiKeypad* Keypad = Cast<AMultiKeypad>(InteractActor))
	{
		Keypad->StartProgress(this);
	}
}

void AEHSurvivorCharacter::ServerRPCCancelInteract_Implementation()
{
	if (AGenerators* Generator = Cast<AGenerators>(InteractActor))
	{
		Generator->CancelRepair(this);
		UE_LOG(LogTemp, Log, TEXT("Generator Cancel"));
	}
	if (AMultiKeypad* Keypad = Cast<AMultiKeypad>(InteractActor))
	{
		Keypad->StopProgress(this);
		UE_LOG(LogTemp, Log, TEXT("Keypad Cancel"));
	}
	InteractActor = nullptr;
	bIsInteracting = false;

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	MulticastStopInteractMontage();
}

void AEHSurvivorCharacter::ServerRPCInteract_Implementation()
{
	StartInteraction();
}

void AEHSurvivorCharacter::OnRep_SetInteractUI()
{
	if (InteractActor && InteractProgressWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("InteractUI AddToViewport!"));
		InteractProgressWidget->AddToViewport();
		GetWorldTimerManager().SetTimer(InteractUIUpdateTimerHandle,this,&AEHSurvivorCharacter::UpdateProgressUI,0.2f,true);
	}
	if (!InteractActor && InteractProgressWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("InteractUI Remove!"));
		InteractProgressWidget->RemoveFromParent();
		GetWorldTimerManager().ClearTimer(InteractUIUpdateTimerHandle);
	}
}

void AEHSurvivorCharacter::UpdateProgressUI()
{

	if (AGenerators* NowActor = Cast<AGenerators>(InteractActor))
	{
		float Progress = NowActor->GetRepairProgress() * 0.01f;
		InteractProgressWidget->SetProgress(Progress);
	}
	if (AMultiKeypad* NowActor = Cast<AMultiKeypad>(InteractActor))
	{
		float Progress = NowActor->GetKeypadProgress() * 0.01f;
		InteractProgressWidget->SetProgress(Progress);
	}
}

//======================================================================

//============================== 상태변환 ==============================

void AEHSurvivorCharacter::UpdateSurvivorState(ESurvivorState NewState)
{
	if (AInGamePlayerState* PS = GetPlayerState<AInGamePlayerState>())
	{
		PS->ServerSetSurvivorState(NewState);
	}
}

void AEHSurvivorCharacter::DeleteHitMark()
{
	GetMesh()->SetCustomDepthStencilValue(2);
}


//============================== 공격 ==============================
void AEHSurvivorCharacter::AttackInput(const FInputActionValue& value) // 공격 인풋
{
	if (bIsVaulting) return;
	if (true == bCanAttack && GetCharacterMovement()->IsFalling() == false)
	{
		ServerRPCAttack();
	}
	if (bIsInteracting) ServerRPCCancelInteract();
}

void AEHSurvivorCharacter::ServerRPCAttack_Implementation() // 서버 공격(서버에서 실행)
{
	// 서버에서 공격 판정
	ServerAttackTrace();
	// 애니메이션 동기화
	MulticastRPCAttack();

	// 공격 중 이동 불가
	bCanAttack = false;
	OnRep_CanAttack();

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([&]()
	{
		bCanAttack = true;
		OnRep_CanAttack();
	}), AttackMontagePlayTime - 0.8f, false);
}

void AEHSurvivorCharacter::MulticastRPCAttack_Implementation() // 클라에서 애니메이션 재생만
{
	PlayAttackMontage();
}

void AEHSurvivorCharacter::ServerAttackTrace_Implementation() // 공격판정 서버에서만
{
	if (!bCanAttack) return;
	
	FVector Start = GetActorLocation() + FVector(0, 0, -30);
	FVector Forward = GetActorForwardVector();
	FVector End = Start + (Forward * 150.f);

	FCollisionShape Sphere = FCollisionShape::MakeSphere(50.f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f, 0, 3.0f);
	DrawDebugSphere(GetWorld(), End, 50.f, 12, FColor::Blue, false, 2.0f);
	
	if (GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_WorldDynamic, Sphere, Params))
	{
		if (AChest* Chest = Cast<AChest>(HitResult.GetActor()))
		{
			UGameplayStatics::ApplyDamage(Chest, 10.f, GetController(), this, nullptr);
		}
	}
}

void AEHSurvivorCharacter::OnRep_CanAttack() // 이동 불가/해제
{
	if (bCanAttack == true)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_None);
	}
}

void AEHSurvivorCharacter::PlayAttackMontage() // 애니메이션 재생
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) == true)
	{
		AnimInstance->StopAllMontages(0.f);
		AnimInstance->Montage_Play(AttackMontage, 1.3f);
	}
}
//======================================================================

// Item : 임시 디버그용 
void AEHSurvivorCharacter::Debug_TakeDamage()
{
	if (!AttributeComp) return;
	AttributeComp->ApplyDamage(50.f); // 테스트값: 50
	Debug_PrintHealth();
}

void AEHSurvivorCharacter::Debug_PrintHealth() const
{
	if (!AttributeComp) return;
	// 접근자 이름은 실제 컴포넌트에 맞게 사용(예: GetHealth()/GetMaxHealth())
	const float Cur = AttributeComp->GetHealth();
	const float Max = AttributeComp->MaxHealth;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			/*Key*/ -1, /*Time*/ 1.5f, FColor::Yellow,
			FString::Printf(TEXT("[HP] %.0f / %.0f"), Cur, Max));
	}
}
void AEHSurvivorCharacter::UseSlot0()
{
	if (UInventoryComponent* Inv = FindComponentByClass<UInventoryComponent>())
	{
		ServerUseConsumableSlot(0); // 1번 키 → 슬롯 0
	}
}

void AEHSurvivorCharacter::UseSlot1()
{
	if (UInventoryComponent* Inv = FindComponentByClass<UInventoryComponent>())
	{
		ServerUseConsumableSlot(1); // 2번 키 → 슬롯 1
	}
}
void AEHSurvivorCharacter::ServerRTryOpenDoorWithKey_Implementation(AMultiplayDoor* Door)
{
	if (Door)
	{
		Door->InterActionDoor(); // 서버에서만 실행됨
	}
}
void AEHSurvivorCharacter::ServerUseConsumableSlot_Implementation(int32 SlotIndex)
{
	if (UInventoryComponent* Inv = FindComponentByClass<UInventoryComponent>())
	{
		const bool bOk = Inv->UseItemBySlot(SlotIndex);
		if (!bOk)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Use] slot %d 사용 실패(빈 슬롯/조건 불만족)"), SlotIndex);
		}
	}
}
