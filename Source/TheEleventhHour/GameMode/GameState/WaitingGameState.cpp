#include "GameMode/GameState/WaitingGameState.h"
#include "GameMode/PlayerState/InGamePlayerState.h" // ✅ 통합 PlayerState 사용
#include "Net/UnrealNetwork.h"

AWaitingGameState::AWaitingGameState()
{
	ReadyCount = 0;
	TotalPlayers = 0;
}

void AWaitingGameState::UpdateReadyState()
{
	int32 NewReady = 0;
	int32 NewTotal = PlayerArray.Num();

	for (APlayerState* PS : PlayerArray)
	{
		if (AInGamePlayerState* WPS = Cast<AInGamePlayerState>(PS))
		{
			if (WPS->bIsReady)
				NewReady++;
		}
	}

	ReadyCount = NewReady;
	TotalPlayers = NewTotal;

	UE_LOG(LogTemp, Log, TEXT("[GameState] UpdateReadyState -> Ready: %d / Total: %d"), ReadyCount, TotalPlayers);

	// ✅ 클라이언트 UI 갱신용 이벤트 브로드캐스트
	OnPlayerCountUpdated.Broadcast();

	// ✅ 모든 인원이 Ready면 자동으로 게임 시작
	if (HasAuthority() && ReadyCount == TotalPlayers && TotalPlayers > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[Server] All players ready! Starting game automatically..."));
		GetWorld()->ServerTravel(TEXT("/Game/Levels/Maps/IngameLevel"));
	}
}

void AWaitingGameState::OnRep_ReadyCount()
{
	OnPlayerCountUpdated.Broadcast();
}

void AWaitingGameState::OnRep_TotalPlayers()
{
	OnPlayerCountUpdated.Broadcast();
}

void AWaitingGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ✅ Always notify even if same value (UI 즉시 갱신 보장)
	DOREPLIFETIME_CONDITION_NOTIFY(AWaitingGameState, ReadyCount, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AWaitingGameState, TotalPlayers, COND_None, REPNOTIFY_Always);
}
