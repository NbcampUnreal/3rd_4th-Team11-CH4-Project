#include "GameMode/GameState/InGameGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

AInGameGameState::AInGameGameState()
{
	bReplicates = true;
}

/* ===================== 서버 전용 호출 ===================== */

void AInGameGameState::IncrementLoadedPlayers()
{
	// 서버에서만 유효
	if (!HasAuthority()) return;

	LoadedPlayers++;
	OnRep_LoadedPlayers(); // 서버에서도 즉시 로그/검증
}

void AInGameGameState::SetGeneratorsRepaired(int32 NewCount)
{
	if (!HasAuthority()) return;

	NewCount = FMath::Clamp(NewCount, 0, 5);

	if (GeneratorsRepaired != NewCount)
	{
		GeneratorsRepaired = NewCount;
		UE_LOG(LogTemp, Log, TEXT("[GS] Generators updated: %d / 5"), GeneratorsRepaired);
	}
}


void AInGameGameState::OpenExitGate()
{
	if (!HasAuthority()) return;

	if (!bExitGateOpened)
	{
		bExitGateOpened = true;
		UE_LOG(LogTemp, Warning, TEXT("[GS] Exit gate opened! Survivors can escape."));
	}
}

void AInGameGameState::SetRemainingTime(int32 NewTime)
{
	if (!HasAuthority()) return;

	const int32 Old = RemainingTime;
	RemainingTime = FMath::Max(NewTime, 0);

	if (Old != RemainingTime)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GS] Remaining time set to %d"), RemainingTime);
	}
}

void AInGameGameState::EndGame(const FString& Reason)
{
	if (!HasAuthority()) return;

	if (!bGameEnded)
	{
		bGameEnded = true;
		EndReason = Reason;
		UE_LOG(LogTemp, Warning, TEXT("[GS] Game Ended: %s"), *EndReason);
		// 추가: 여기서 GM이 세션 정리/리턴 맵 이동을 트리거해도 됨
	}
}

/* ===================== OnRep: 클라 UI 토스트/로그 ===================== */

void AInGameGameState::OnRep_LoadedPlayers()
{
	UE_LOG(LogTemp, Log, TEXT("[GS] LoadedPlayers: %d / %d"), LoadedPlayers, ExpectedPlayers);

	// 서버에서만 “모두 로딩 완료” 브로드캐스트
	if (HasAuthority() && ExpectedPlayers > 0 && LoadedPlayers >= ExpectedPlayers)
	{
		UE_LOG(LogTemp, Log, TEXT("[GS] All players loaded! Broadcasting event."));
		OnAllPlayersLoaded.Broadcast();
	}
}

/* ===================== Net ===================== */

void AInGameGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInGameGameState, ExpectedPlayers);
	DOREPLIFETIME(AInGameGameState, LoadedPlayers);
	DOREPLIFETIME(AInGameGameState, RemainingTime);
	DOREPLIFETIME(AInGameGameState, GeneratorsRepaired);
	DOREPLIFETIME(AInGameGameState, bExitGateOpened);
	DOREPLIFETIME(AInGameGameState, bGameEnded);
	DOREPLIFETIME(AInGameGameState, EndReason);
}
