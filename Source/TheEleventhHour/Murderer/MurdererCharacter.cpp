// Fill out your copyright notice in the Description page of Project Settings.


#include "Murderer/MurdererCharacter.h"

#include "EngineUtils.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Survivor/Character/EHSurvivorCharacter.h"
#include "components/AudioComponent.h"
#include "Map/BackgroundMusicActor.h"
#include "Murderer/OutlineManager/SurvivorOutlineManager.h"
#include "Survivor/OutlineManager/SurvivorVisibilityManager.h"

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

	// 추격 음악 컴포넌트 생성
	ChaseMusicAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("ChaseMusicAudio"));
	ChaseMusicAudioComp->SetupAttachment(RootComponent);
	ChaseMusicAudioComp->bAutoActivate = false;

	bIsKiller=true;
}

void AMurdererCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	

	//----------플레이어 추격 음악 로직-----------
    if (!IsLocallyControlled() || !ChaseMusicAudioComp || !BGMActor) 
        return;
    
    //가장 가까운 생존자 찾기
    AEHSurvivorCharacter* ClosestSurvivor = nullptr;
    float MinDistance = MAX_FLT;
    
    for (TActorIterator<AEHSurvivorCharacter> It(GetWorld()); It; ++It)
    {
        AEHSurvivorCharacter* Survivor = *It;
        if (!Survivor || Survivor->bIsDead) continue;
        
        float Distance = FVector::Dist(Survivor->GetActorLocation(), GetActorLocation());
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestSurvivor = Survivor;
        }
    }
    
    //거리 기반 추격 음악 재생 제어
    const float MaxRange = 4000.0f;
    const float MinRange = 1500.0f;
    
    if (!ClosestSurvivor || MinDistance > MaxRange)
    {
        //놓치면 추격 음악 정지, BGM 원래 볼륨
        if (ChaseMusicAudioComp->IsPlaying())
        {
            ChaseMusicAudioComp->Stop();
        }
        
        if (BGMActor->AudioComponent)
        {
            BGMActor->AudioComponent->SetVolumeMultiplier(1.0f);
        }
        return;
    }
    
    //범위 안에 있으면 추격 음악 재생 시작
    if (!ChaseMusicAudioComp->IsPlaying())
    {
        ChaseMusicAudioComp->Play();
    }
    
    //거리 기반 볼륨 계산
    float ChaseVolume = FMath::GetMappedRangeValueClamped(
        FVector2D(MaxRange, MinRange), 
        FVector2D(0.0f, 1.f),  // 멀리 있으면 0, 가까우면 1.5
        MinDistance
    );
    
    //BGM 볼륨은 추격 음악과 반비례
    float BGMVolume = FMath::GetMappedRangeValueClamped(
        FVector2D(MaxRange, MinRange),
        FVector2D(1.0f, 0.f),  // 멀리 있으면 1.0, 가까우면 0.1
        MinDistance
    );
    
    //피치 (생존자처럼 거리에 따라 변화)
    float ChasePitch = FMath::GetMappedRangeValueClamped(
        FVector2D(MaxRange, MinRange),
        FVector2D(1.f, 1.f),
        MinDistance
    );
    
    ChaseMusicAudioComp->SetVolumeMultiplier(ChaseVolume);
    ChaseMusicAudioComp->SetPitchMultiplier(ChasePitch);
    
    if (BGMActor->AudioComponent)
    {
        BGMActor->AudioComponent->SetVolumeMultiplier(BGMVolume);
    }
	//-------------------------------------------------------------
}

// Called when the game starts or when spawned
void AMurdererCharacter::BeginPlay()
{
	Super::BeginPlay();
	HitActorsThisSwing.Reset();

	for (TActorIterator<ASurvivorOutlineManager> It(GetWorld());It;++It)
	{
		OutlineManager=*It;
	}

	//BackgroundMusicActor 찾아서 캐싱
	for (TActorIterator<ABackgroundMusicActor> It(GetWorld()); It; ++It)
	{
		BGMActor = *It;
		break;
	}
    
	//추격 음악 사운드 설정
	if (ChaseMusic && ChaseMusicAudioComp)
	{
		ChaseMusicAudioComp->SetSound(ChaseMusic);
	}
}

void AMurdererCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass,bIsSprint);
	DOREPLIFETIME(ThisClass,SprintGauge);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass,bCanSkill1Use,COND_OwnerOnly,REPNOTIFY_OnChanged)
}

void AMurdererCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	if (!IsLocallyControlled()) return;
	if (OutlineManager)
	{
		OutlineManager->AfterPossess();
	}
	else if (!OutlineManager)
	{
		for (TActorIterator<ASurvivorOutlineManager> It(GetWorld());It;++It)
		{
			OutlineManager = *It;
		}
		OutlineManager->AfterPossess();
	}

	if (ASurvivorVisibilityManager* tmp = Cast<ASurvivorVisibilityManager>(UGameplayStatics::GetActorOfClass(GetWorld(),ASurvivorVisibilityManager::StaticClass())))
	{
		tmp->bIsLocalSurvivor=false;
	}
}


void AMurdererCharacter::Server_PlayFootstepSound_Implementation()
{
	Multicast_PlayFootstepSound();
}

void AMurdererCharacter::Multicast_PlayFootstepSound_Implementation()
{
	if (HasAuthority() || !FootStepSound) return;

	FVector CurrentCharacterLoc=GetActorLocation();

	UGameplayStatics::PlaySoundAtLocation(
		this,
		FootStepSound,
		CurrentCharacterLoc,
		1.f,
		1.f,
		0.f);
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
	tmp->BindAction(Skill1Action,ETriggerEvent::Started,this,&ThisClass::Skill1);
	tmp->BindAction(JumpAction,ETriggerEvent::Started,this,&ThisClass::Jump);

}

void AMurdererCharacter::Skill1_Implementation()
{
	if (bCanSkill1Use)
	{
		bCanSkill1Use=false;
		//30초 쿨다운
		GetWorldTimerManager().SetTimer(
			Skill1CollDownTimerHandle,
			[this]()
			{
				bCanSkill1Use=true;
			},
			Skill1CooldownDuration,
			false
			);
	}
}
void AMurdererCharacter::OnRep_bCanSkill1Use()
{
	if (IsLocallyControlled()) // 살인마가 아니면 얼리 엑시트 시키기
	{
		if (bCanSkill1Use==false) // 서버에서 스킬 사용한 뒤 레플리케이션 됐다면
		{
			if (OutlineManager)
			{
				RemainSkill1Cooldown=Skill1CooldownDuration;
				OutlineManager->SetSurvivorOverlayTransparent();

				GetWorldTimerManager().SetTimer(
					Skill1DurationTimerHandle,
					[this]()
					{
						//5초만 생존자 전체 위치 노출
						OutlineManager->DisableSurvivorOverlayTransparent();
					},
					Skill1Duration,
					false);

				//스킬 쿨다운 설정
				GetWorldTimerManager().SetTimer(
					Skill1RemainCoolDownHandle,
					[this]()
					{
						RemainSkill1Cooldown=FMath::Clamp(RemainSkill1Cooldown-0.1f,0.f,Skill1CooldownDuration);
						if (RemainSkill1Cooldown<=KINDA_SMALL_NUMBER)
						{
							RemainSkill1Cooldown=0.0f;
							GetWorldTimerManager().ClearTimer(Skill1RemainCoolDownHandle);
						}
					},
					0.1f,
					true,
					0.1f);
			}
		}
		else if (bCanSkill1Use==true)
		{
			GetWorldTimerManager().ClearTimer(Skill1RemainCoolDownHandle);
			RemainSkill1Cooldown=0.0f;
		}
	}
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

void AMurdererCharacter::Jump()
{
	Super::Jump();
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
	GetWorldTimerManager().ClearTimer(Skill1CollDownTimerHandle);
	GetWorldTimerManager().ClearTimer(Skill1DurationTimerHandle);
	GetWorldTimerManager().ClearTimer(Skill1RemainCoolDownHandle);
	GetWorldTimerManager().ClearTimer(AttackCooldownHandle);
}

void AMurdererCharacter::Attack_Implementation(const FInputActionValue& ActionValue)
{
	if (bCanAttack)
	{
		NetMulticast_PlayAttackAnimMontage();
		
		bCanAttack=false;
		
		GetWorldTimerManager().SetTimer(
		AttackTimerHandle,
		[this]()
		{
			bCanAttack=true;
		},
		5.0f,
		false);
	}
}

void AMurdererCharacter::NetMulticast_PlayAttackAnimMontage_Implementation()
{
		//UE_LOG(LogTemp,Log,TEXT("공격 몽타주 재생 시도"));
		if (AttackMontage && GetMesh()->GetAnimInstance())
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (!AnimInstance->Montage_IsPlaying(AttackMontage))
			{
				AnimInstance->Montage_Play(AttackMontage);
				//UE_LOG(LogTemp,Log,TEXT("공격 몽타주 재생 성공"));
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
		RemainAttackCooldown=5.0f;
		GetWorldTimerManager().SetTimer(
			AttackCooldownHandle,
			[this]()
			{
				RemainAttackCooldown=FMath::Clamp(RemainAttackCooldown-0.1f,0,5.0f);
				if (RemainAttackCooldown<=KINDA_SMALL_NUMBER)
				{
					GetWorldTimerManager().ClearTimer(AttackCooldownHandle);
				}
			},
			0.1f,
			true,
			0.1f);
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
	//UE_LOG(LogTemp,Log,TEXT(__FUNCTION__));
}
