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
#include "Item/Chest/Chest.h"
#include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Item/Inventory/InventoryComponent.h"
#include "Net/UnrealNetwork.h"

AEHSurvivorCharacter::AEHSurvivorCharacter()
	:InterActionLength(300.0f)
	, bCanAttack(true)
	, AttackMontagePlayTime(0.f)
	, DeathMontagePlayTime(0.f)
	, InteractActor(nullptr)
	, bIsCrouching(false)
	, bIsInteracting(false)
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

	if (IsValid(AttackMontage) == true)
	{
		AttackMontagePlayTime = AttackMontage->GetPlayLength();
	}
	if (IsValid(DeathMontage) == true)
	{
		DeathMontagePlayTime = DeathMontage->GetPlayLength();
	}
	
	if (IsLocallyControlled() == true)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		checkf(IsValid(PC) == true, TEXT("PlayerController is invalid."));
	
		UEnhancedInputLocalPlayerSubsystem* EILPS = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
		checkf(IsValid(EILPS) == true, TEXT("EnhancedInputLocalPlayerSubsystem is invalid."));
	
		EILPS->AddMappingContext(InputMappingContext, 0);
	}
}

void AEHSurvivorCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bCanAttack);
	DOREPLIFETIME(ThisClass, bIsInteracting);
	DOREPLIFETIME(ThisClass, InteractActor);
}

//============================== 피격, 죽음 ==============================
void AEHSurvivorCharacter::MulticastHitMontage_Implementation()
{
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
		// 죽는 모션 먼저 재생 후 처리 코드
		Destroy();
	}
}
//======================================================================

//============================== 데미지 처리 ==============================
float AEHSurvivorCharacter::TakeDamage(float DamageAmount,
									   const FDamageEvent& DamageEvent,
									   AController* EventInstigator,
									   AActor* DamageCauser)
{
	if (!HasAuthority())
		return 0.f;
	
	if (bIsInteracting) ServerRPCCancelInteract();
	
	if (AttributeComp)
	{
		const float CurrentHealth = AttributeComp->GetHealth();
		AttributeComp->TakeDamage(DamageAmount);
		const float NewHealth = AttributeComp->GetHealth();

		if (NewHealth <= 0.f && !bIsDead)
		{
			bIsDead = true;
			MulticastDeathMontage();
		}
		else
		{
			MulticastHitMontage();
		}
		return CurrentHealth - NewHealth;
	}
	return 0.f;
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
	EIC->BindAction(WalkAction, ETriggerEvent::Started, this, &ThisClass::Walk);
	EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &ThisClass::AttackInput);
	EIC->BindAction(InteractAction,ETriggerEvent::Started,this,&AEHSurvivorCharacter::StartInteraction);
}
//======================================================================

//============================== 이동, 시점 ==============================
void AEHSurvivorCharacter::Move(const FInputActionValue& value)
{
	if (IsValid(Controller) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("Controller is invalid."));
		return;
	}
	
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
		UE_LOG(LogTemp, Error, TEXT("Controller is invalid."));
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
		UE_LOG(LogTemp, Warning, TEXT("UnCrouch!"));
	}
	else
	{
		Crouch();
		UE_LOG(LogTemp, Warning, TEXT("Crouch!"));
	}
}
//======================================================================

//============================== 파쿠르(예정) ==============================
void AEHSurvivorCharacter::StartCross(const FInputActionValue& value)
{
}

void AEHSurvivorCharacter::EndCross(const FInputActionValue& value)
{
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
					bIsInteracting = true;
					InteractActor = Generator;
					ServerRPCStartInteract();
				}
			}
			if (AMultiKeypad* Keypad = Cast<AMultiKeypad>(HitActor))
			{
				if (!bIsInteracting)
				{
					bIsInteracting = true;
					InteractActor = Keypad;
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

void AEHSurvivorCharacter::ServerRPCInteractionDoorOnPawn_Implementation(class AMultiplayDoor* Door)
{
	Door->InterActionDoor();
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
		Keypad->StartProgress();
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
		Keypad->StopProgress();
		UE_LOG(LogTemp, Log, TEXT("Keypad Cancel"));
	}
	InteractActor = nullptr;
	bIsInteracting = false;
}

void AEHSurvivorCharacter::ServerRPCInteract_Implementation()
{
	StartInteraction();
}
//======================================================================

//============================== 테스트용 ==============================
void AEHSurvivorCharacter::Walk(const FInputActionValue& value)
{
	
}
//======================================================================

//============================== 공격 ==============================
void AEHSurvivorCharacter::AttackInput(const FInputActionValue& value) // 공격 인풋
{
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

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f, 0, 3.0f);
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldDynamic, Params))
	{
		if (AChest* Chest = Cast<AChest>(HitResult.GetActor()))
		{
			UGameplayStatics::ApplyDamage(Chest, 10.f, GetController(), this, nullptr);
			UE_LOG(LogTemp, Log, TEXT("Chest Attack"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Attack miss!"));
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
