#include "GameMode/WaitingGameMode.h"
#include "GameMode/GameState/WaitingGameState.h"
#include "GameMode/PlayerState/WaitingPlayerState.h"
#include "Net/UnrealNetwork.h"

void AWaitingGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// 플레이어가 접속하면 PlayerState 확인
	if (AWaitingPlayerState* PS = NewPlayer->GetPlayerState<AWaitingPlayerState>())
	{
		// 🔥 서버(방장)는 자동으로 Ready
		if (PS->HasAuthority())  
		{
			PS->bIsReady = true;

			// GameState에 즉시 반영
			if (AWaitingGameState* GS = GetGameState<AWaitingGameState>())
			{
				GS->UpdateReadyState();
				GS->OnPlayerCountUpdated.Broadcast();
			}
		}
	}
}

void AWaitingGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	AWaitingGameState* GS = GetGameState<AWaitingGameState>();
	if (GS)
	{
		GS->UpdateReadyState();
	}
}

void AWaitingGameMode::StartGame()
{
	AWaitingGameState* GS = GetGameState<AWaitingGameState>();
	if (!GS) return;

	// 모든 플레이어가 준비 완료했는지 체크
	if (GS->ReadyCount == GS->TotalPlayers && GS->TotalPlayers > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[Server] All players ready! Starting game..."));
		GetWorld()->ServerTravel(NextLevelName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server] Cannot start: Ready %d / Total %d"),
			   GS->ReadyCount, GS->TotalPlayers);
	}
}