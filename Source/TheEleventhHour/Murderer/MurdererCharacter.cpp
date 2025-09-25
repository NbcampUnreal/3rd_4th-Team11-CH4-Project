// Fill out your copyright notice in the Description page of Project Settings.


#include "Murderer/MurdererCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AMurdererCharacter::AMurdererCharacter()
	:BaseSpeed(600.0f)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	DamageTypeClass = UDamageType::StaticClass();
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw=false;
	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f); 
	
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
	SpringArmComponent->TargetArmLength = 400.0f;
	SpringArmComponent -> bUsePawnControlRotation = true;
	SpringArmComponent -> SetupAttachment(RootComponent);
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent -> SetupAttachment(SpringArmComponent,USpringArmComponent::SocketName);
	CameraComponent-> bUsePawnControlRotation = false;
}

// Called when the game starts or when spawned
void AMurdererCharacter::BeginPlay()
{
	Super::BeginPlay();
	HitActorsThisSwing.Reset();
}

// Called to bind functionality to input
void AMurdererCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (IsLocallyControlled())
	{
		
		APlayerController* PC = Cast<APlayerController>(GetController());
		PC->SetInputMode(FInputModeGameOnly());
		
		UEnhancedInputLocalPlayerSubsystem* EILPS = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
		EILPS->AddMappingContext(InputMappingContext,0);
	}
	
	UEnhancedInputComponent* tmp = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	tmp->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
	tmp->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
	tmp->BindAction(AttackAction,ETriggerEvent::Triggered,this, &ThisClass::Attack);
	
}

void AMurdererCharacter::Move(const FInputActionValue& InputValue)
{
	FVector2D Vec= InputValue.Get<FVector2D>();

	FRotator ControllerRotation = FRotator(0,Controller->GetControlRotation().Yaw,0);
	FVector FowardDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::X);
	FVector RightDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(FowardDirection,Vec.X);
	AddMovementInput(RightDirection,Vec.Y);
}

void AMurdererCharacter::Look(const FInputActionValue& ActionValue)
{
	FVector2D Vec = ActionValue.Get<FVector2D>();

	AddControllerYawInput(Vec.X);
	AddControllerPitchInput(Vec.Y);
}

void AMurdererCharacter::Attack(const FInputActionValue& ActionValue)
{
	if (AttackMontage && GetMesh()->GetAnimInstance())
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		UE_LOG(LogTemp,Log,TEXT("sdasd123"));
		if (!AnimInstance->Montage_IsPlaying(AttackMontage))
		{
			AnimInstance->Montage_Play(AttackMontage);
			UE_LOG(LogTemp,Log,TEXT("sdasd"));
		}
	}
}

void AMurdererCharacter::BeginMeleeSwingWindow()
{
	HitActorsThisSwing.Reset();

}

void AMurdererCharacter::PerformMeleeHit()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const FVector TraceStart = GetActorLocation() + FVector(0, 0, 50.f);
	const FVector TraceEnd   = TraceStart + GetActorForwardVector() * MeleeRange;

	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(MeleeSweep), false, this);

	const bool bHit = World->SweepMultiByChannel(
		HitResults,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		TraceChannel,
		FCollisionShape::MakeSphere(MeleeRadius),
		Params
	);

	if (bDrawDebug)
	{
		DrawDebugLine(World, TraceStart, TraceEnd, FColor::Green, false, 1.5f, 0, 2.f);
		DrawDebugSphere(World, TraceEnd, MeleeRadius, 16, bHit ? FColor::Red : FColor::Blue, false, 1.5f);
	}

	if (!bHit) return;

	const FVector ShotDir = (TraceEnd - TraceStart).GetSafeNormal();

	for (const FHitResult& HR : HitResults)
	{
		AActor* HitActor = HR.GetActor();
		if (!IsValid(HitActor) || HitActor == this) continue;

		// 같은 스윙에서 중복 타격 방지
		if (HitActorsThisSwing.Contains(HitActor)) continue;

		// 팀/아군 판정 필요 시 여기서 필터링(ActorHasTag 등)

		UGameplayStatics::ApplyPointDamage(
			HitActor,
			MeleeDamage,
			ShotDir,
			HR,
			GetController(),
			this,
			DamageTypeClass
		);

		HitActorsThisSwing.Add(HitActor);

		// 선택: 이펙트/사운드 
	}
}

