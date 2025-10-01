#include "Survivor/Character/EHSurvivorCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "gimmick/Door/MultiplayDoor.h"
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
	, Health(MaxHealth)
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

	bIsCrouching = false;
	bIsInteracting = false;
	bIsWalking = false;
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
}

//--------피격&죽음----------
void AEHSurvivorCharacter::MulticastHitMontage_Implementation()
{
	if (Health <= 0.f) return;
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
		AnimInstance->OnMontageEnded.AddDynamic(this, &AEHSurvivorCharacter::OnDeathMontageEnded);
		AnimInstance->Montage_Play(DeathMontage);
		GetMesh()->bPauseAnims = true;
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
//----------------------

float AEHSurvivorCharacter::TakeDamage(float DamageAmount,
									   const FDamageEvent& DamageEvent,
									   AController* EventInstigator,
									   AActor* DamageCauser)
{
	if (!HasAuthority())
		return 0.f;

	const float ActualDamage = FMath::Max(0.f, DamageAmount);
	if (ActualDamage <= 0.f)
		return 0.f;

	// 체력 감소
	const float OldHealth = Health;
	Health = FMath::Clamp(Health - ActualDamage, 0.f, MaxHealth);
	
	Super::TakeDamage(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
	
	MulticastHitMontage();
	
	// 사망 처리
	if (Health <= 0.f)
	{
		bIsDead = true;
		MulticastDeathMontage();
		//Destroy(); // 서버에서 파괴 → 클라이언트에도 복제됨
	}
	
	// 실제로 적용된 데미지 반환(남은 체력보다 큰 데미지면 깎인 만큼만)
	return OldHealth - Health;
}

void AEHSurvivorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}

// ------인풋 바인딩------
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
// ------------------

// ------이동, 마우스------
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
// ------------------

// -------앉기-------
void AEHSurvivorCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	bIsCrouching = true;
	
	if (SpringArm)
	{
		SpringArm->SetRelativeLocation(CrouchSpringArmLocation);
	}
}
void AEHSurvivorCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	bIsCrouching = false;
	
	if (SpringArm)
	{
		SpringArm->SetRelativeLocation(DefaultSpringArmLocation);
	}
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
// ------------------

// ------파쿠르------
void AEHSurvivorCharacter::StartCross(const FInputActionValue& value)
{
}

void AEHSurvivorCharacter::EndCross(const FInputActionValue& value)
{
}
// ------------------

// ------상호작용------
void AEHSurvivorCharacter::StartInteraction(const FInputActionValue& value)
{
	bIsInteracting = true;

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
				ServerRPCInteractionDoorOnPawn(Door);
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
// ------------------

// -------뛰기 애니메이션 테스트-------
void AEHSurvivorCharacter::Walk(const FInputActionValue& value)
{
	
}
// ------------------

// -------공격-------
void AEHSurvivorCharacter::AttackInput(const FInputActionValue& value)
{
	if (true == bCanAttack && GetCharacterMovement()->IsFalling() == false)
	{
		ServerRPCAttack();
	}
}

void AEHSurvivorCharacter::ServerRPCAttack_Implementation()
{
	MulticastRPCAttack();
}

bool AEHSurvivorCharacter::ServerRPCAttack_Validate()
{
	return true;
}

void AEHSurvivorCharacter::MulticastRPCAttack_Implementation()
{
	if (HasAuthority() == true)
	{
		bCanAttack = false;
		
		OnRep_CanAttack();
		
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([&]()
		{
			bCanAttack = true;
			OnRep_CanAttack();
		}), AttackMontagePlayTime - 0.8f, false);
	}

	PlayAttackMontage();
}

void AEHSurvivorCharacter::OnRep_CanAttack()
{
	if (bCanAttack == true)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_None);
		
		FVector Start = GetActorLocation() + FVector(0, 0, -30);
		FVector Forward = GetActorForwardVector();
		FVector End = Start + (Forward * 150.f);

		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f, 0, 3.0f);
		if (GetWorld()->LineTraceSingleByChannel(HitResult,Start,End,ECC_WorldDynamic,Params))
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
}

void AEHSurvivorCharacter::PlayAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) == true)
	{
		AnimInstance->StopAllMontages(0.f);
		AnimInstance->Montage_Play(AttackMontage, 1.3f);
	}
}
//------------------

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
		Inv->UseItemBySlot(0); // 1번 키 → 슬롯 0
	}
}

void AEHSurvivorCharacter::UseSlot1()
{
	if (UInventoryComponent* Inv = FindComponentByClass<UInventoryComponent>())
	{
		Inv->UseItemBySlot(1); // 2번 키 → 슬롯 1
	}
}