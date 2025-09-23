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

AEHSurvivorCharacter::AEHSurvivorCharacter()
	:InterActionLength(300.0f)
{
	PrimaryActorTick.bCanEverTick = true;

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
	
	if (IsLocallyControlled() == true)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		checkf(IsValid(PC) == true, TEXT("PlayerController is invalid."));
	
		UEnhancedInputLocalPlayerSubsystem* EILPS = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
		checkf(IsValid(EILPS) == true, TEXT("EnhancedInputLocalPlayerSubsystem is invalid."));
	
		EILPS->AddMappingContext(InputMappingContext, 0);
	}
}

void AEHSurvivorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// ------인풋 바인딩------
void AEHSurvivorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
	EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
	EIC->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::ToggleCrouch);
	EIC->BindAction(WalkAction, ETriggerEvent::Started, this, &ThisClass::Walk);
	EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &ThisClass::AttackInput);
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
				Door->InterActionDoor();
			}
		}
	}
	else
	{
		UE_LOG(LogTemp,Log,TEXT("LineTraceMiss"));
	}
}
// ------------------

// -------걷기-------
void AEHSurvivorCharacter::Walk(const FInputActionValue& value)
{
	if (bIsCrouching)
	{
		return;
	}
	
	bIsWalking = !bIsWalking;
	GetCharacterMovement()->MaxWalkSpeed = bIsWalking ? 400.f : 600.f;
}
// ------------------

// -------공격-------
void AEHSurvivorCharacter::Attack()
{
	UE_LOG(LogTemp, Warning, TEXT("Attack!"));

	if (!HasAuthority())
	{
		ServerAttack();
		return;
	}

	FVector Start = GetActorLocation() + FVector(0, 0, -30);
	FVector Forward = GetActorForwardVector();
	FVector End = Start + (Forward * 300.f);

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
void AEHSurvivorCharacter::AttackInput(const FInputActionValue& value)
{
	Attack();
}

void AEHSurvivorCharacter::ServerAttack_Implementation()
{
	Attack();
}
// ------------------
