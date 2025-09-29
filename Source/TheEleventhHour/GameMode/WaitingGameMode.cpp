#include "GameMode/WaitingGameMode.h"
#include "GameMode/GameState/WaitingGameState.h"
#include "GameMode/PlayerState/WaitingPlayerState.h"
#include "Net/UnrealNetwork.h"

void AWaitingGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    // 첫 번째 접속자(Host)만 자동 Ready
    if (GetNumPlayers() == 1)
    {
        if (AWaitingPlayerState* PS = NewPlayer->GetPlayerState<AWaitingPlayerState>())
        {
            PS->bIsReady = true;

            if (AWaitingGameState* GS = GetGameState<AWaitingGameState>())
            {
                GS->UpdateReadyState();
                GS->OnPlayerCountUpdated.Broadcast(); // UI 갱신
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