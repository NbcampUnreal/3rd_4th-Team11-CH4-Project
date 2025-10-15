#include "GameMode/Prison/PrisonManager.h"
#include "GameMode/InGameGameMode.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "Survivor/Character/EHSurvivorCharacter.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

APrisonManager::APrisonManager()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;

	// 🔹 EscapeTrigger는 BP에서 설정할 수 있게 생성만
	EscapeTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("EscapeTrigger"));
	EscapeTrigger->SetupAttachment(RootComponent);
	EscapeTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EscapeTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	EscapeTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	EscapeTrigger->SetGenerateOverlapEvents(true);
}

void APrisonManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		LinkedGameMode = GetWorld()->GetAuthGameMode<AInGameGameMode>();

		if (EscapeTrigger)
			EscapeTrigger->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnEscapeOverlapBegin);

		if (!EscapeSpot)
			EscapeSpot = FindEscapeSpot();
	}
}

/* -------------------- 감옥 포획 -------------------- */
void APrisonManager::Capture(AEHSurvivorCharacter* Victim)
{
	if (!HasAuthority() || !Victim) return;

	if (!CapturedSurvivors.Contains(Victim))
	{
		CapturedSurvivors.Add(Victim);

		AActor* PrisonSpot = nullptr;
		for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
		{
			if (It->PlayerStartTag == FName("Spawn.Prison"))
			{
				PrisonSpot = *It;
				break;
			}
		}

		const FVector Loc = PrisonSpot ? PrisonSpot->GetActorLocation() : GetActorLocation();
		const FRotator Rot = PrisonSpot ? PrisonSpot->GetActorRotation() : GetActorRotation();
		Victim->TeleportTo(Loc, Rot, false, true);

		if (AInGamePlayerState* PS = Victim->GetPlayerState<AInGamePlayerState>())
			PS->ServerSetSurvivorState(ESurvivorState::Captured);

		// ✅ 통계 업데이트 (서버만)
		if (AInGamePlayerState* VictimPS = Victim->GetPlayerState<AInGamePlayerState>())
		{
			VictimPS->AddCapturedTime();
		}

		// ✅ 킬러 통계도 추가
		if (AInGameGameMode* GM = GetWorld()->GetAuthGameMode<AInGameGameMode>())
		{
			for (APlayerState* PS : GM->GameState->PlayerArray)
			{
				if (AInGamePlayerState* KillerPS = Cast<AInGamePlayerState>(PS))
				{
					if (KillerPS->bIsKiller)
					{
						KillerPS->AddKillerCapture();
						break;
					}
				}
			}
		}
		
		UE_LOG(LogTemp, Warning, TEXT("[PrisonManager] %s captured and teleported."), *Victim->GetName());
	}
}

/* -------------------- 트리거 통과 시 -------------------- */
void APrisonManager::OnEscapeOverlapBegin(
	UPrimitiveComponent* OverlappedComp, 
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	// 🔹 서버 전용
	if (!HasAuthority())
		return;

	AEHSurvivorCharacter* Survivor = Cast<AEHSurvivorCharacter>(OtherActor);
	if (!Survivor)
		return;

	AInGamePlayerState* PS = Survivor->GetPlayerState<AInGamePlayerState>();
	if (!PS)
		return;

	// 🔹 감옥에 있던 플레이어만 부상 상태로 전환
	if (PS->SurvivorState == ESurvivorState::Captured)
	{
		PS->ServerSetSurvivorState(ESurvivorState::Injured);

		Survivor->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		Survivor->EnableInput(nullptr);

		UE_LOG(LogTemp, Warning, TEXT("[PrisonManager] %s escaped from prison → set to INJURED."), *Survivor->GetName());
	}
}



/* -------------------- 문 닫힘 시 강제 이동 -------------------- */
void APrisonManager::ForceMoveCapturedToEscape()
{
	if (!HasAuthority() || CapturedSurvivors.Num() == 0)
		return;

	if (!EscapeSpot)
		EscapeSpot = FindEscapeSpot();

	if (!EscapeSpot)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PrisonManager] No Spawn.Escape found!"));
		return;
	}

	for (AEHSurvivorCharacter* Prisoner : CapturedSurvivors)
	{
		if (!IsValid(Prisoner)) continue;

		if (AInGamePlayerState* PS = Prisoner->GetPlayerState<AInGamePlayerState>())
		{
			if (PS->SurvivorState == ESurvivorState::Captured)
			{
				Prisoner->TeleportTo(EscapeSpot->GetActorLocation(), EscapeSpot->GetActorRotation(), false, true);
				PS->ServerSetSurvivorState(ESurvivorState::Injured);

				Prisoner->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
				Prisoner->EnableInput(nullptr);

				UE_LOG(LogTemp, Warning, TEXT("[PrisonManager] %s moved to EscapeSpawn and set to Injured."), *Prisoner->GetName());
			}
		}
	}

	CapturedSurvivors.Empty();
}

/* -------------------- EscapeSpot 찾기 -------------------- */
AActor* APrisonManager::FindEscapeSpot()
{
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (It->PlayerStartTag == FName("Spawn.Escape"))
			return *It;
	}
	return nullptr;
}

/* -------------------- 네트워크 복제 -------------------- */
void APrisonManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APrisonManager, CapturedSurvivors);
}
