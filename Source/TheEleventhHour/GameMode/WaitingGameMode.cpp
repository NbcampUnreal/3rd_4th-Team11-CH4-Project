#include "GameMode/WaitingGameMode.h"
#include "GameState/WaitingGameState.h"
#include "PlayerState/InGamePlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Character.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

AWaitingGameMode::AWaitingGameMode()
{
	bUseSeamlessTravel = true;

	GameStateClass = AWaitingGameState::StaticClass();
	PlayerStateClass = AInGamePlayerState::StaticClass(); // Waiting 단계에서도 InGamePlayerState 사용
}

void AWaitingGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 맵에 배치된 모든 PlayerStart를 찾아서 배열에 저장
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (APlayerStart* PS = *It)
		{
			SpawnPoints.Add(PS);
		}
	}

	//UE_LOG(LogTemp, Log, TEXT("[WaitingGM] Found %d PlayerStart spawn points"), SpawnPoints.Num());
}

void AWaitingGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// 기존 Pawn은 엔진이 이미 SpawnDefaultPawnFor 로 생성해줌.
	// 여기서 다시 SpawnDefaultPawnFor 호출하지 말 것!

	if (ACharacter* Char = Cast<ACharacter>(NewPlayer->GetPawn()))
	{
		// 움직임 막고 대기 상태 처리만
		Char->DisableInput(NewPlayer);
		//UE_LOG(LogTemp, Log, TEXT("[WaitingGM] Existing Pawn %s found for %s"),
		//	*Char->GetName(), *NewPlayer->GetName());
	}

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
			//UE_LOG(LogTemp, Log, TEXT("[Server] All players ready! Starting game..."));

			// 🔹 다음 맵으로 이동 시 플레이어 수를 GameState에 저장
			for (APlayerState* PS : GameState->PlayerArray)
			{
				if (AInGamePlayerState* InPS = Cast<AInGamePlayerState>(PS))
				{
					InPS->bIsReady = false; // Ready 초기화
				}
			}

			// 🔹 SeamlessTravel 사용
			bUseSeamlessTravel = true;
			GetWorld()->ServerTravel(TEXT("/Game/Levels/Maps/IngameLevel"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Server] Cannot start: Ready %d / Total %d"),
				GS->ReadyCount, GS->TotalPlayers);
		}
	}
}

