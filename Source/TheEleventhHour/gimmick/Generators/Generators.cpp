#include "gimmick/Generators/Generators.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameMode/InGameGameMode.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "Survivor/Character/EHSurvivorCharacter.h"

AGenerators::AGenerators()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SetRootComponent(SceneComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	MeshComponent->SetupAttachment(SceneComponent);
	MeshComponent->SetRenderCustomDepth(true);
	MeshComponent->SetCustomDepthStencilValue(4);
	
	BoxComponent = CreateDefaultSubobject<UBoxComponent>("BoxComponent");
	BoxComponent->SetupAttachment(SceneComponent);
	BoxComponent->SetCollisionProfileName(TEXT("Trigger"));

	AudioComponent = CreateDefaultSubobject<UAudioComponent>("AudioComponent");
	
	VibrateValue = 0.15f;
	RepairValue = 0.1f;
	RepairProgress = 0.0f;
	RepairDuration = 0.05f;
	RepairCount = 0;
	bIsStartRepairing = false;
	bIsCompleted = false;
	RepairEfficiency = { 0.f, 1.f, 1.8f, 2.4f, 2.8f };

	//Replicate 활성화
	bReplicates = true;

	bAlwaysRelevant = true;
}

void AGenerators::StartRepair(AEHSurvivorCharacter* Survivor)
{
	//서버에서만 실행
	if (!HasAuthority()) return;
	
	//수리 완료 여부, 참여 인원 등 유효성 검사
	if (bIsCompleted) return;
	if (RepairCount >= 4) return;

	//수리 참여 인원 추가, 수리 타이머 핸들이 없을 경우 초기에 한번만 생성
	if (Survivor && !InteractSurvivors.Contains(Survivor))
	{
		InteractSurvivors.Add(Survivor);
		UE_LOG(LogTemp, Log, TEXT("[UI_SIMPLE] Added Survivor: %s (Total: %d)"), 
			   *Survivor->GetName(), InteractSurvivors.Num());
	}
	RepairCount++;
	if (RepairTimerHandle.IsValid()) return;
	GetWorldTimerManager().SetTimer(RepairTimerHandle,this,&AGenerators::RepairGenerator,RepairDuration,true);

	//발전기를 처음 수리하였을 때 true
	if (bIsStartRepairing) return;
	bIsStartRepairing = true;
}

void AGenerators::CancelRepair(AEHSurvivorCharacter* Survivor)
{
	//서버에서만 실행
	if (!HasAuthority()) return;
	
	//수리 참여 인원 감소
	if (Survivor)
	{
		InteractSurvivors.Remove(Survivor);
		UE_LOG(LogTemp, Log, TEXT("[UI_SIMPLE] Removed Survivor: %s (Total: %d)"), 
			   *Survivor->GetName(), InteractSurvivors.Num());
	}
	RepairCount--;
}

void AGenerators::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
}

//복제 대상 목록
void AGenerators::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
	DOREPLIFETIME(AGenerators, RepairProgress);
	DOREPLIFETIME(AGenerators, RepairCount);
	DOREPLIFETIME(AGenerators, bIsStartRepairing);
	DOREPLIFETIME(AGenerators, bIsCompleted);
	DOREPLIFETIME(AGenerators, InteractSurvivors);
}

void AGenerators::VibrateAnimation()
{
	//진동 좌표 설정
	float RandomX = FMath::FRandRange(0.f,0.1f);
	float RandomY = FMath::FRandRange(0.f,0.1f);
	float RandomZ = FMath::FRandRange(0.f,VibrateValue);

	float NewX = StartLocation.X + RandomX;
	float NewY = StartLocation.Y + RandomY;
	float NewZ = StartLocation.Z + RandomZ;

	FVector VibLocation = FVector(NewX,NewY,NewZ);
	//액터 이동(진동)
	SetActorLocation(VibLocation);
}

void AGenerators::RepairGenerator()
{
	//서버만 해당 로직을 실행함
	if (!HasAuthority()) return;
	
	//진동 애니메이션(MulticastRPC)
	MulticastRPCVibrateAnimation();

	//수리 로직
	RepairProgress += RepairEfficiency[RepairCount] * RepairValue;
	
	//디버깅 메시지
	MulticastRPCShowRepairProgress();
	
	//수리가 완료될 경우
	if (RepairProgress >= 100.f)
	{
		//수리 완료(MulticastRPC)
		MulticastRPCCompleteGenerator();
	}
}

void AGenerators::CompleteGenerator()
{
	if (HasAuthority())
	{
		TArray<AEHSurvivorCharacter*> SurvivorsCopy = InteractSurvivors;
		
		for (AEHSurvivorCharacter* Survivor : SurvivorsCopy)
		{
			if (IsValid(Survivor))
			{
				// ✅ 수리 중단 처리
				Survivor->ServerRPCCancelInteract();

				// ✅ 통계 반영: 발전기 수리 횟수 +1
				if (AInGamePlayerState* PS = Survivor->GetPlayerState<AInGamePlayerState>())
				{
					PS->AddGeneratorRepair();
					UE_LOG(LogTemp, Log, TEXT("[Generator] %s repaired generator (total: %d)"),
						*PS->GetPlayerName(), PS->NumGeneratorsRepaired);

					if (AInGameGameMode* GM = GetWorld()->GetAuthGameMode<AInGameGameMode>())
					{
						GM->LogAllPlayerStates();
					}
				}
			}
		}

		// ✅ 게임모드에 전체 발전기 수리 보고
		if (AInGameGameMode* GM = Cast<AInGameGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GM->CompletedRepair();
		}

		RepairCount = -1;
	}

	UGameplayStatics::PlaySoundAtLocation(this, CompleteSound, GetActorLocation());
	MulticastRPCStopGeneratorSound();
	GetWorldTimerManager().ClearTimer(RepairTimerHandle);
	bIsCompleted = true;
}

void AGenerators::MulticastRPCVibrateAnimation_Implementation()
{
	VibrateAnimation();
}


void AGenerators::MulticastRPCCompleteGenerator_Implementation()
{
	CompleteGenerator();
}

//IsStartRepairing(발전기 동작 여부) 감지하여 사운드 재생
void AGenerators::OnRep_PlayingSound()
{
	if (bIsStartRepairing && !bIsCompleted)
	{
		AudioComponent = UGameplayStatics::SpawnSoundAtLocation(this, GeneratorSound, GetActorLocation());
	}
}

void AGenerators::OnRep_RepairCount()
{
	if (MeshComponent)
	{
		if (RepairCount > 0)
		{
			MeshComponent->SetRenderCustomDepth(true);
			MeshComponent->SetCustomDepthStencilValue(3);
		}
		else if (RepairCount == 0)
		{
			MeshComponent->SetRenderCustomDepth(true);
			MeshComponent->SetCustomDepthStencilValue(4);
		}
		else if (RepairCount == -1)
		{
			MeshComponent->SetRenderCustomDepth(false);
			MeshComponent->SetCustomDepthStencilValue(0);
		}
	}
}

void AGenerators::MulticastRPCStopGeneratorSound_Implementation()
{
	AudioComponent->Stop();
}

//디버깅용
void AGenerators::MulticastRPCShowRepairProgress_Implementation()
{
	if (GEngine)
	{
		FString DebugMsg = FString::Printf(TEXT("RepairProgress: %.2f, RepairCount: %d"), RepairProgress, RepairCount);
		GEngine->AddOnScreenDebugMessage(-1, RepairDuration - 0.01f, FColor::Green, DebugMsg);
	}
}

