// Fill out your copyright notice in the Description page of Project Settings.


#include "Murderer/MurdererCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AMurdererCharacter::AMurdererCharacter()
	:BaseSpeed(600.0f),SprintSpeed(900.0f),SprintGauge(MaxSprintGauge)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates=true;
	SetReplicateMovement(true);
	
	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent -> SetupAttachment(RootComponent);
	CameraComponent -> bUsePawnControlRotation = true;
	CameraComponent -> SetMobility(EComponentMobility::Type::Movable);

	PivotForMesh1P = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
	PivotForMesh1P -> SetupAttachment(CameraComponent);
	PivotForMesh1P -> bInheritPitch=false;
	PivotForMesh1P -> bInheritYaw=true;
	PivotForMesh1P -> bInheritRoll=true;
	PivotForMesh1P->TargetArmLength=0.0f;
	
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh1P");
	Mesh1P -> SetupAttachment(PivotForMesh1P);
	
	DamageTypeClass = UDamageType::StaticClass();
}

void AMurdererCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			666, 1.0f, FColor::Green,
			FString::Printf(TEXT("SprintGauge: %f"), SprintGauge)
		);
	}
}

// Called when the game starts or when spawned
void AMurdererCharacter::BeginPlay()
{
	Super::BeginPlay();
	HitActorsThisSwing.Reset();
	
}

void AMurdererCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass,bIsSprint);
	DOREPLIFETIME(ThisClass,SprintGauge);
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
	tmp->BindAction(AttackAction,ETriggerEvent::Started,this, &ThisClass::Attack);
	tmp->BindAction(SprintAction,ETriggerEvent::Started,this,&ThisClass::StartSprint);
	tmp->BindAction(SprintAction,ETriggerEvent::Completed,this,&ThisClass::EndSprint);
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

void AMurdererCharacter::StartSprint_Implementation()
{
	if (SprintGauge<=10)
		return;

	GetWorldTimerManager().ClearTimer(SprintEndTimerHandle);
	
	GetCharacterMovement()->MaxWalkSpeed=SprintSpeed;
	bIsSprint=true;
	
	GetWorldTimerManager().SetTimer(
		SprintStartTimerHandle
		,[this]()
		{
			SprintGauge= FMath::Clamp(--SprintGauge,0,MaxSprintGauge);
			
			if (SprintGauge<=0)
			{
				GetWorldTimerManager().ClearTimer(SprintStartTimerHandle);
				EndSprint_Implementation();
			}
		}
		,0.1f
		,true
		,0.0f);
}


void AMurdererCharacter::EndSprint_Implementation()
{
	if (!bIsSprint)
		return;
	GetWorldTimerManager().ClearTimer(SprintStartTimerHandle);

	GetCharacterMovement()->MaxWalkSpeed=BaseSpeed;
	bIsSprint=false;

	if (SprintGauge>=MaxSprintGauge)
		return;
	
	GetWorldTimerManager().SetTimer(
		SprintEndTimerHandle,
		[this]()
		{
			SprintGauge=FMath::Clamp(++SprintGauge,0,MaxSprintGauge);

			if (SprintGauge>=MaxSprintGauge)
			{
				GetWorldTimerManager().ClearTimer(SprintEndTimerHandle);
			}
		}
		,0.25f
		,true
		,5.0f
		);
}

void AMurdererCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	// 수명 종료 시 안전 정리 (람다에서 this 캡처 중이므로 필수)
	GetWorldTimerManager().ClearTimer(SprintStartTimerHandle);
	GetWorldTimerManager().ClearTimer(SprintEndTimerHandle);
}

void AMurdererCharacter::Attack_Implementation(const FInputActionValue& ActionValue)
{
	NetMulticast_PlayAttackAnimMontage();
}

void AMurdererCharacter::NetMulticast_PlayAttackAnimMontage_Implementation()
{

		UE_LOG(LogTemp,Log,TEXT("공격 몽타주 재생 시도"));
		if (AttackMontage && GetMesh()->GetAnimInstance())
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (!AnimInstance->Montage_IsPlaying(AttackMontage))
			{
				AnimInstance->Montage_Play(AttackMontage);
				UE_LOG(LogTemp,Log,TEXT("공격 몽타주 재생 성공"));
			}
		}
		if (IsLocallyControlled()&&Mesh1P&&Mesh1P->GetAnimInstance())
		{
			UAnimInstance* Anim = Mesh1P->GetAnimInstance();
			if (AttackMontage && !Anim->Montage_IsPlaying(AttackMontage))
			{
				Anim->Montage_Play(AttackMontage);
			}
		}
}
void AMurdererCharacter::BeginMeleeSwingWindow()
{
	if (HasAuthority())
		HitActorsThisSwing.Reset();
}

void AMurdererCharacter::OnRep_bIsSprint()
{
	if (bIsSprint)
		GetCharacterMovement()->MaxWalkSpeed=SprintSpeed;
	else if (!bIsSprint)
		GetCharacterMovement()->MaxWalkSpeed=BaseSpeed;
}

void AMurdererCharacter::PerformMeleeHit()
{
	if (HasAuthority())
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
	UE_LOG(LogTemp,Log,TEXT(__FUNCTION__));

}


