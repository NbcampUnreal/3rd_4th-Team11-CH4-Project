#include "GameMode/PlayerState/InGamePlayerState.h"
#include "GameMode/GameState/WaitingGameState.h"
#include "GameMode/InGameGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

AInGamePlayerState::AInGamePlayerState()
{
	bIsReady = false;
	bIsKiller = false;
	SurvivorState = ESurvivorState::Alive;

	bAlwaysRelevant = true;
}

/* -------------------- Ready 상태 -------------------- */
void AInGamePlayerState::ServerSetReady_Implementation(bool bNewReady)
{
	if (bIsReady == bNewReady)
		return;

	bIsReady = bNewReady;
	OnRep_bIsReady();

	// ✅ 대기실 상태 업데이트 (WaitingGameState)
	UpdateGameStateReadyCount();
}

void AInGamePlayerState::OnRep_bIsReady()
{
	UE_LOG(LogTemp, Log, TEXT("[PS] %s Ready -> %d"), *GetPlayerName(), bIsReady);
	UpdateGameStateReadyCount();
}

/* -------------------- Killer -------------------- */
void AInGamePlayerState::OnRep_bIsKiller()
{
	UE_LOG(LogTemp, Log, TEXT("[PS] %s Role -> %s"),
		*GetPlayerName(),
		bIsKiller ? TEXT("KILLER") : TEXT("SURVIVOR"));
}

/* -------------------- Survivor 상태 -------------------- */
void AInGamePlayerState::ServerSetSurvivorState_Implementation(ESurvivorState NewState)
{
	if (SurvivorState == NewState)
		return; // 중복 방지

	SurvivorState = NewState;
	OnRep_SurvivorState();

	// ✅ 인게임 상태 변화 → GameMode에 알림
	NotifyGameModeStateChanged();

	// ✅ 감옥 이동 처리 (서버 전용)
	if (HasAuthority() && NewState == ESurvivorState::Captured)
	{
		if (AInGameGameMode* GM = GetWorld()->GetAuthGameMode<AInGameGameMode>())
		{
			GM->SendToPrison(this);
			UE_LOG(LogTemp, Warning, TEXT("[PS] %s has been captured → SendToPrison() called."), *GetPlayerName());
		}
	}
}

void AInGamePlayerState::OnRep_SurvivorState()
{
	const FString StateName = StaticEnum<ESurvivorState>()->GetNameStringByValue((int64)SurvivorState);
	UE_LOG(LogTemp, Warning, TEXT("[PS] %s SurvivorState -> %s"), *GetPlayerName(), *StateName);

	// 🔹 HUD/UI 반영 가능 (Delegate or Widget Update)
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan,
			FString::Printf(TEXT("%s 상태: %s"), *GetPlayerName(), *StateName));
	}
}

/* -------------------- Waiting 단계 GameState 갱신 -------------------- */
void AInGamePlayerState::UpdateGameStateReadyCount()
{
	if (!GetWorld()) return;

	if (HasAuthority())
	{
		if (AWaitingGameState* GS = GetWorld()->GetGameState<AWaitingGameState>())
		{
			GS->UpdateReadyState();
			UE_LOG(LogTemp, Log, TEXT("[PS] (%s) UpdateGameStateReadyCount() called"), *GetPlayerName());
		}
	}
}

/* -------------------- InGame 단계 GameMode 통보 -------------------- */
void AInGamePlayerState::NotifyGameModeStateChanged()
{
	if (HasAuthority())
	{
		if (AInGameGameMode* GM = GetWorld()->GetAuthGameMode<AInGameGameMode>())
		{
			GM->EvaluateAllPlayerStates();
			GM->CheckVictoryConditions();
			UE_LOG(LogTemp, Log, TEXT("[PS] (%s) NotifyGameModeStateChanged() → EvaluateAllPlayerStates()"), *GetPlayerName());
		}
	}
}


/* -------------------- Replication -------------------- */
void AInGamePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInGamePlayerState, bIsReady);
	DOREPLIFETIME(AInGamePlayerState, bIsKiller);
	DOREPLIFETIME(AInGamePlayerState, SurvivorState);
}
