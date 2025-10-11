#include "GameMode/InGameGameMode.h"
#include "GameMode/GameState/InGameGameState.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "gimmick/Keypad/MultiKeypad.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Engine/World.h"

AInGameGameMode::AInGameGameMode()
:GeneratorCount(1)
{
	bUseSeamlessTravel = true;

	GameStateClass = AInGameGameState::StaticClass();
	PlayerStateClass = AInGamePlayerState::StaticClass();
}

void AInGameGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (AInGameGameState* GS = GetGameState<AInGameGameState>())
		{
			GS->ExpectedPlayers = GameState->PlayerArray.Num();
			GS->OnAllPlayersLoaded.AddDynamic(this, &AInGameGameMode::OnAllPlayersLoaded);
			UE_LOG(LogTemp, Log, TEXT("[InGameGM] ExpectedPlayers = %d"), GS->ExpectedPlayers);
		}
	}
}

void AInGameGameMode::OnAllPlayersLoaded()
{
	UE_LOG(LogTemp, Log, TEXT("[InGameGM] All players loaded! Starting match countdown."));
	AssignRoles();
	StartCountdown();
}


void AInGameGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	if (AInGameGameState* GS = GetGameState<AInGameGameState>())
	{
		GS->ExpectedPlayers = GameState->PlayerArray.Num();
		UE_LOG(LogTemp, Log, TEXT("[InGameGM] PostSeamlessTravel - ExpectedPlayers = %d"), GS->ExpectedPlayers);
	}
}

void AInGameGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (HasAuthority() && !bMatchStarted)
	{
		TryStartMatchWhenAllLoaded();
	}
}

void AInGameGameMode::TryStartMatchWhenAllLoaded()
{
	AInGameGameState* GS = GetGameState<AInGameGameState>();
	if (!GS) return;

	if (GS->LoadedPlayers >= GS->ExpectedPlayers && GS->ExpectedPlayers > 0)
	{
		bMatchStarted = true;

		UE_LOG(LogTemp, Log, TEXT("[InGameGM] All players loaded (%d/%d). Starting countdown."),
			GS->LoadedPlayers, GS->ExpectedPlayers);

		AssignRoles();
		StartCountdown();
	}
}

#pragma region --- ROLE ASSIGNMENT ---

void AInGameGameMode::AssignRoles()
{
	const TArray<APlayerState*>& Players = GameState->PlayerArray;
	if (Players.Num() == 0) return;

	int32 KillerIndex = UKismetMathLibrary::RandomInteger(Players.Num());

	for (int32 i = 0; i < Players.Num(); ++i)
	{
		if (AInGamePlayerState* PS = Cast<AInGamePlayerState>(Players[i]))
		{
			PS->bIsKiller = (i == KillerIndex);
			PS->ForceNetUpdate();
			UE_LOG(LogTemp, Log, TEXT("[RoleAssign] %s -> %s"),
				*PS->GetPlayerName(),
				PS->bIsKiller ? TEXT("KILLER") : TEXT("SURVIVOR"));
		}
	}
}

#pragma endregion

#pragma region --- COUNTDOWN & PAWN SPAWN ---

void AInGameGameMode::StartCountdown()
{
	CountdownTime = 5;
	GetWorld()->GetTimerManager().SetTimer(
		CountdownTimer, this,
		&AInGameGameMode::CountdownTick, 1.0f, true);
}

void AInGameGameMode::CountdownTick()
{
	UE_LOG(LogTemp, Log, TEXT("[InGameGM] Match starts in %d"), CountdownTime);

	if (CountdownTime <= 1)
	{
		GetWorld()->GetTimerManager().ClearTimer(CountdownTimer);
		ReplaceKillerPawn();
		StartMatchTimer();
		UE_LOG(LogTemp, Log, TEXT("[InGameGM] Countdown finished. Match begins."));
	}

	CountdownTime--;
}

void AInGameGameMode::ReplaceKillerPawn()
{
	UE_LOG(LogTemp, Warning, TEXT("[ReplaceKillerPawn] Start"));

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		AInGamePlayerState* PS = Cast<AInGamePlayerState>(PC->PlayerState);
		if (!PS) continue;

		// Survivor는 기본 Pawn 유지
		if (!PS->bIsKiller)
			continue;

		UE_LOG(LogTemp, Warning, TEXT("[ReplaceKillerPawn] Found Killer: %s"), *PS->GetPlayerName());

		if (APawn* OldPawn = PC->GetPawn())
			OldPawn->Destroy();

		FVector Loc = FVector::ZeroVector;
		FRotator Rot = FRotator::ZeroRotator;
		if (AActor* StartSpot = FindPlayerStart(PC))
		{
			Loc = StartSpot->GetActorLocation();
			Rot = StartSpot->GetActorRotation();
		}

		FActorSpawnParameters Params;
		Params.Owner = PC;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APawn* NewPawn = GetWorld()->SpawnActor<APawn>(KillerPawnClass, Loc, Rot, Params);
		if (NewPawn)
		{
			PC->Possess(NewPawn);
			UE_LOG(LogTemp, Warning, TEXT("[ReplaceKillerPawn] %s now possesses %s"),
				*PS->GetPlayerName(), *NewPawn->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[ReplaceKillerPawn] Failed to spawn KillerPawn"));
		}
	}
}

#pragma endregion

#pragma region --- MATCH TIMER & VICTORY LOGIC ---

void AInGameGameMode::StartMatchTimer()
{
	GetWorld()->GetTimerManager().SetTimer(MatchTimerHandle, this,
		&AInGameGameMode::MatchTick, MatchTickInterval, true);
}

void AInGameGameMode::MatchTick()
{
	AInGameGameState* GS = GetGameState<AInGameGameState>();
	if (!GS || GS->bGameEnded) return;

	GS->RemainingTime--;

	// 탈출구가 열리면 60초 이내로 유지
	if (GS->bExitGateOpened && GS->RemainingTime > 60)
	{
		GS->RemainingTime = 60;
	}

	if (GS->RemainingTime <= 0)
	{
		EndGame(TEXT("Time’s up! Survivors failed to escape."));
		return;
	}

	CheckVictoryConditions();
}

void AInGameGameMode::CheckVictoryConditions()
{
	AInGameGameState* GS = GetGameState<AInGameGameState>();
	if (!GS || GS->bGameEnded) return;

	int32 Alive = 0, Escaped = 0, Dead = 0;
	for (APlayerState* PS : GS->PlayerArray)
	{
		if (AInGamePlayerState* Player = Cast<AInGamePlayerState>(PS))
		{
			switch (Player->SurvivorState)
			{
				case ESurvivorState::Alive:
				case ESurvivorState::Injured:
					Alive++; break;
				case ESurvivorState::Escaped:
					Escaped++; break;
				case ESurvivorState::Dead:
				case ESurvivorState::Captured:
					Dead++; break;
			}
		}
	}

	// 발전기 완료 → 탈출구 열기
	if (GS->GeneratorsRepaired >= 5 && !GS->bExitGateOpened)
	{
		GS->OpenExitGate();
		UE_LOG(LogTemp, Warning, TEXT("[InGameGM] Exit gate opened! Survivors can escape."));
	}

	// 모든 생존자 사망 → 킬러 승
	if (Alive == 0)
	{
		EndGame(TEXT("Killer wins! All survivors are dead."));
	}

	// 모든 생존자 탈출 → 생존자 승
	if (Escaped > 0 && Alive == 0)
	{
		EndGame(TEXT("Survivors escaped successfully!"));
	}
}

void AInGameGameMode::EndGame(const FString& Reason)
{
	AInGameGameState* GS = GetGameState<AInGameGameState>();
	if (!GS || GS->bGameEnded) return;

	GS->bGameEnded = true;
	GS->EndReason = Reason;

	GetWorld()->GetTimerManager().ClearTimer(MatchTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(CountdownTimer);

	UE_LOG(LogTemp, Warning, TEXT("[InGameGM] GAME OVER: %s"), *Reason);
}

#pragma endregion

void AInGameGameMode::CompletedRepair()
{
    GeneratorCount--;
    UE_LOG(LogTemp, Log, TEXT("[InGameGM] CompletedRepair -> GeneratorCount: %d"), GeneratorCount);
    
    if (GeneratorCount <= 0)
    {
        for (TActorIterator<AMultiKeypad> It(GetWorld()); It; ++It)
        {
            AMultiKeypad* Keypad = *It;
            if (Keypad)
            {
                Keypad->SetCanInteract(true);
                UE_LOG(LogTemp, Log, TEXT("[InGameGM] CompletedRepair -> SetCanInteract(true)"));
            }
        }
    }
}
