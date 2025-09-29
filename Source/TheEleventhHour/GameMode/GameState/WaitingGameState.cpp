#include "GameMode/GameState/WaitingGameState.h"
#include "GameMode/PlayerState/WaitingPlayerState.h"
#include "Net/UnrealNetwork.h"

AWaitingGameState::AWaitingGameState()
{
	ReadyCount = 0;
	TotalPlayers = 0;
}

void AWaitingGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWaitingGameState, ReadyCount);
	DOREPLIFETIME(AWaitingGameState, TotalPlayers);
}

void AWaitingGameState::UpdateReadyState()
{
	if (HasAuthority())
	{
		int32 NewReady = 0;
		int32 NewTotal = PlayerArray.Num();

		for (APlayerState* PS : PlayerArray)
		{
			if (AWaitingPlayerState* WPS = Cast<AWaitingPlayerState>(PS))
			{
				if (WPS->bIsReady)
				{
					NewReady++;
				}
			}
		}

		ReadyCount = NewReady;
		TotalPlayers = NewTotal;

		OnPlayerCountUpdated.Broadcast(); // 서버에서도 즉시 UI 갱신 요청
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