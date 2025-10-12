#include "GameMode/Prison/PrisonManager.h"
#include "GameMode/InGameGameMode.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "Survivor/Character/EHSurvivorCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"

APrisonManager::APrisonManager()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
}

/* -------------------- BeginPlay -------------------- */
void APrisonManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		LinkedGameMode = GetWorld()->GetAuthGameMode<AInGameGameMode>();

		// 자동 EscapeSpot 탐색
		if (!EscapeSpot)
		{
			EscapeSpot = FindEscapeSpot();
		}

		if (EscapeSpot)
		{
			UE_LOG(LogTemp, Log, TEXT("[PrisonManager] EscapeSpot found: %s"), *EscapeSpot->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PrisonManager] ❌ No EscapeSpot found (Tag = Spawn.Escape)"));
		}
	}
}

/* -------------------- 생존자 수감 -------------------- */
void APrisonManager::Capture(AEHSurvivorCharacter* Victim)
{
	if (!HasAuthority() || !Victim) return;

	if (!CapturedSurvivors.Contains(Victim))
	{
		CapturedSurvivors.Add(Victim);

		// 감옥 위치로 이동
		FVector PrisonLoc = GetActorLocation() + FVector(0, 0, 50.f);
		Victim->TeleportTo(PrisonLoc, GetActorRotation(), false, true);

		// 이동 및 입력 제한
		Victim->DisableInput(nullptr);
		Victim->GetCharacterMovement()->DisableMovement();

		// 상태 변경
		if (AInGamePlayerState* PS = Victim->GetPlayerState<AInGamePlayerState>())
		{
			PS->ServerSetSurvivorState(ESurvivorState::Captured);
		}

		UE_LOG(LogTemp, Warning, TEXT("[Prison] %s captured and moved to prison."), *Victim->GetName());
	}
}

/* -------------------- 구출 시작 -------------------- */
void APrisonManager::StartRescue_Implementation(AEHSurvivorCharacter* Rescuer)
{
	if (!HasAuthority() || CapturedSurvivors.Num() == 0)
		return;

	// 중복 방지
	if (CurrentRescuer && CurrentRescuer != Rescuer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Prison] Rescue already in progress by %s."), *CurrentRescuer->GetName());
		return;
	}

	CurrentRescuer = Rescuer;
	UE_LOG(LogTemp, Log, TEXT("[Prison] Rescue started by %s"), *Rescuer->GetName());

	if (LinkedGameMode)
		LinkedGameMode->NotifyRescueStarted(Rescuer);

	// RescueDuration 초 후 완료
	GetWorldTimerManager().SetTimer(RescueTimer, this, &APrisonManager::CompleteRescue, RescueDuration, false);
}

/* -------------------- 구출 취소 -------------------- */
void APrisonManager::CancelRescue_Implementation(AEHSurvivorCharacter* Rescuer)
{
	if (!HasAuthority()) return;
	if (CurrentRescuer != Rescuer) return;

	GetWorldTimerManager().ClearTimer(RescueTimer);
	CurrentRescuer = nullptr;

	UE_LOG(LogTemp, Log, TEXT("[Prison] Rescue canceled by %s"), *Rescuer->GetName());
}

/* -------------------- 구출 완료 -------------------- */
void APrisonManager::CompleteRescue()
{
	if (!HasAuthority())
		return;

	UE_LOG(LogTemp, Warning, TEXT("[Prison] Rescue complete! Freeing all survivors..."));

	FreeAllSurvivors();

	if (LinkedGameMode)
		LinkedGameMode->NotifyRescueCompleted();

	// 리셋
	CurrentRescuer = nullptr;
}

/* -------------------- 수감자 해방 -------------------- */
void APrisonManager::FreeAllSurvivors()
{
	for (AEHSurvivorCharacter* Prisoner : CapturedSurvivors)
	{
		if (!IsValid(Prisoner))
			continue;

		// 상태 Alive로 복구
		if (AInGamePlayerState* PS = Prisoner->GetPlayerState<AInGamePlayerState>())
		{
			PS->ServerSetSurvivorState(ESurvivorState::Alive);
		}

		// 이동 가능 복구
		Prisoner->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		Prisoner->EnableInput(nullptr);

		// 탈출 위치로 이동
		if (EscapeSpot)
		{
			FVector EscapeLoc = EscapeSpot->GetActorLocation();
			FRotator EscapeRot = EscapeSpot->GetActorRotation();
			Prisoner->TeleportTo(EscapeLoc, EscapeRot, false, true);
		}

		UE_LOG(LogTemp, Log, TEXT("[Prison] %s freed from prison."), *Prisoner->GetName());
	}

	CapturedSurvivors.Empty();
}

/* -------------------- EscapeSpot 자동 탐색 -------------------- */
AActor* APrisonManager::FindEscapeSpot()
{
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (It->PlayerStartTag == FName("Spawn.Escape"))
		{
			return *It;
		}
	}
	return nullptr;
}

/* -------------------- Network Replication -------------------- */
void APrisonManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APrisonManager, CapturedSurvivors);
	DOREPLIFETIME(APrisonManager, CurrentRescuer);
}
