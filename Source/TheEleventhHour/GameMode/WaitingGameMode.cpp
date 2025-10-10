#include "GameMode/WaitingGameMode.h"
#include "GameState/WaitingGameState.h"
#include "PlayerState/InGamePlayerState.h"
#include "Net/UnrealNetwork.h"

AWaitingGameMode::AWaitingGameMode()
{
    bUseSeamlessTravel = true;

    GameStateClass = AWaitingGameState::StaticClass();
    PlayerStateClass = AInGamePlayerState::StaticClass(); // Waiting 단계에서도 InGamePlayerState 사용
}

void AWaitingGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    // --- ✅ Killer 결정 로직 ---
    if (HasAuthority())  // 서버에서만 실행
    {
        AInGamePlayerState* PS = NewPlayer->GetPlayerState<AInGamePlayerState>();
        if (PS)
        {
            // 현재 연결된 플레이어 수 확인
            int32 CurrentPlayers = GameState ? GameState->PlayerArray.Num() : 0;

            // 첫 번째 접속자만 Killer
            if (CurrentPlayers == 1)
            {
                PS->bIsKiller = true;
                UE_LOG(LogTemp, Log, TEXT("[WaitingGM] %s set as KILLER (first player)"), *PS->GetPlayerName());
            }
            else
            {
                PS->bIsKiller = false;
                UE_LOG(LogTemp, Log, TEXT("[WaitingGM] %s set as SURVIVOR"), *PS->GetPlayerName());
            }

            PS->ForceNetUpdate(); // ✅ 즉시 클라로 동기화
        }
    }

    // --- ReadyState 갱신 ---
    if (AWaitingGameState* GS = GetGameState<AWaitingGameState>())
    {
        GS->UpdateReadyState();
    }
}

void AWaitingGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    if (AWaitingGameState* GS = GetGameState<AWaitingGameState>())
    {
        GS->UpdateReadyState();
    }
}

void AWaitingGameMode::StartGame()
{
    if (AWaitingGameState* GS = GetGameState<AWaitingGameState>())
    {
        if (GS->ReadyCount == GS->TotalPlayers && GS->TotalPlayers > 0)
        {
            UE_LOG(LogTemp, Log, TEXT("[Server] All players ready! Starting game..."));
            GetWorld()->ServerTravel(TEXT("/Game/Levels/Maps/IngameLevel"), true);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[Server] Cannot start: Ready %d / Total %d"),
                   GS->ReadyCount, GS->TotalPlayers);
        }
    }
}
