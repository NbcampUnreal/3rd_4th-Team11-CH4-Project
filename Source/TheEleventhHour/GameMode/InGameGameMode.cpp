#include "GameMode/InGameGameMode.h"
#include "GameMode/GameState/InGameGameState.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "GameMode/Prison/PrisonManager.h"
#include "Survivor/Character/EHSurvivorCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "gimmick/Keypad/MultiKeypad.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AInGameGameMode::AInGameGameMode()
	: PrisonManager(nullptr), GeneratorCount(1)
{
	bUseSeamlessTravel = true;

	GameStateClass = AInGameGameState::StaticClass();
	PlayerStateClass = AInGamePlayerState::StaticClass();
}

/* -------------------- BeginPlay -------------------- */
void AInGameGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (AInGameGameState* GS = GetGameState<AInGameGameState>())
		{
			GS->ExpectedPlayers = GameState->PlayerArray.Num();
			GS->OnAllPlayersLoaded.AddDynamic(this, &AInGameGameMode::OnAllPlayersLoaded);
			UE_LOG(LogTemp, Log, TEXT("[InGameGM] BeginPlay - ExpectedPlayers = %d"), GS->ExpectedPlayers);
		}
	}

	// ✅ 감옥 매니저 찾기
	for (TActorIterator<APrisonManager> It(GetWorld()); It; ++It)
	{
		PrisonManager = *It;
		break;
	}
	if (PrisonManager)
	{
		UE_LOG(LogTemp, Log, TEXT("[InGameGM] PrisonManager found: %s"), *PrisonManager->GetName());
	}
}

/* -------------------- PostSeamlessTravel -------------------- */
void AInGameGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	if (AInGameGameState* GS = GetGameState<AInGameGameState>())
	{
		GS->ExpectedPlayers = GameState->PlayerArray.Num();
		UE_LOG(LogTemp, Log, TEXT("[InGameGM] PostSeamlessTravel - ExpectedPlayers = %d"), GS->ExpectedPlayers);
	}
}

/* -------------------- Tick -------------------- */
void AInGameGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority() && !bMatchStarted)
	{
		TryStartMatchWhenAllLoaded();
	}
}

AActor* AInGameGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPoints);

	TArray<APlayerStart*> PlayerTaggedStarts;

	// 🔹 "Spawn.Player" 태그가 붙은 스타트만 필터링
	for (AActor* Actor : SpawnPoints)
	{
		APlayerStart* Start = Cast<APlayerStart>(Actor);
		if (Start && Start->PlayerStartTag == FName("Spawn.Player"))
		{
			PlayerTaggedStarts.Add(Start);
		}
	}

	// ✅ 태그가 붙은 스타트가 하나라도 있다면 랜덤 선택
	if (PlayerTaggedStarts.Num() > 0)
	{
		const int32 Index = FMath::RandRange(0, PlayerTaggedStarts.Num() - 1);
		APlayerStart* ChosenStart = PlayerTaggedStarts[Index];
		UE_LOG(LogTemp, Log, TEXT("[ChoosePlayerStart] Spawned at %s"), *ChosenStart->GetName());
		return ChosenStart;
	}

	// 🚨 없으면 기본 로직 사용 (안전 fallback)
	UE_LOG(LogTemp, Warning, TEXT("[ChoosePlayerStart] No 'Spawn.Player' found! Using default."));
	return Super::ChoosePlayerStart_Implementation(Player);
}

/* -------------------- TryStartMatch -------------------- */
void AInGameGameMode::TryStartMatchWhenAllLoaded()
{
	AInGameGameState* GS = GetGameState<AInGameGameState>();
	if (!GS) return;

	if (GS->LoadedPlayers >= GS->ExpectedPlayers && GS->ExpectedPlayers > 0)
	{
		bMatchStarted = true;
		UE_LOG(LogTemp, Log, TEXT("[InGameGM] All players loaded (%d/%d). Starting countdown."), GS->LoadedPlayers, GS->ExpectedPlayers);
		AssignRoles();
		StartCountdown();
	}
}

/* -------------------- OnAllPlayersLoaded -------------------- */
void AInGameGameMode::OnAllPlayersLoaded()
{
	UE_LOG(LogTemp, Log, TEXT("[InGameGM] AllPlayersLoaded -> Countdown start."));
	AssignRoles();
	StartCountdown();
}

/* -------------------- AssignRoles -------------------- */
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
			UE_LOG(LogTemp, Log, TEXT("[RoleAssign] %s -> %s"), *PS->GetPlayerName(), PS->bIsKiller ? TEXT("KILLER") : TEXT("SURVIVOR"));
		}
	}
}

/* -------------------- Countdown -------------------- */
void AInGameGameMode::StartCountdown()
{
	CountdownTime = 5;
	GetWorld()->GetTimerManager().SetTimer(CountdownTimer, this, &AInGameGameMode::CountdownTick, 1.0f, true);
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

/* -------------------- ReplaceKillerPawn -------------------- */
void AInGameGameMode::ReplaceKillerPawn()
{
	UE_LOG(LogTemp, Log, TEXT("[ReplaceKillerPawn] Killer pawn replacement start."));

	APlayerController* KillerPC = nullptr;
	AInGamePlayerState* KillerPS = nullptr;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		AInGamePlayerState* PS = Cast<AInGamePlayerState>(PC->PlayerState);
		if (PS && PS->bIsKiller)
		{
			KillerPC = PC;
			KillerPS = PS;
			break;
		}
	}
	if (!KillerPC || !KillerPS) return;

	FVector SpawnLoc = FVector::ZeroVector;
	FRotator SpawnRot = FRotator::ZeroRotator;

	if (APawn* OldPawn = KillerPC->GetPawn())
	{
		SpawnLoc = OldPawn->GetActorLocation();
		SpawnRot = OldPawn->GetActorRotation();
		OldPawn->Destroy();
	}
	else if (AActor* DefaultStart = FindPlayerStart(KillerPC))
	{
		SpawnLoc = DefaultStart->GetActorLocation();
		SpawnRot = DefaultStart->GetActorRotation();
	}

	FActorSpawnParameters Params;
	Params.Owner = KillerPC;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (APawn* NewPawn = GetWorld()->SpawnActor<APawn>(KillerPawnClass, SpawnLoc, SpawnRot, Params))
	{
		KillerPC->Possess(NewPawn);
	}
}

/* -------------------- Match Timer -------------------- */
void AInGameGameMode::StartMatchTimer()
{
	GetWorld()->GetTimerManager().SetTimer(MatchTimerHandle, this, &AInGameGameMode::MatchTick, MatchTickInterval, true);
}

void AInGameGameMode::MatchTick()
{
	AInGameGameState* GS = GetGameState<AInGameGameState>();
	if (!GS || GS->bGameEnded) return;

	GS->RemainingTime--;
	if (GS->bExitGateOpened && GS->RemainingTime > 60) GS->RemainingTime = 60;

	if (GS->RemainingTime <= 0)
	{
		EndGame(TEXT("Time’s up! Survivors failed to escape."));
		return;
	}
	CheckVictoryConditions();
}

/* -------------------- 감옥 시스템 -------------------- */
void AInGameGameMode::CapturePlayer(AEHSurvivorCharacter* Victim)
{
	if (!PrisonManager || !Victim) return;
	PrisonManager->Capture(Victim);
	EvaluateAllPlayerStates();
}

void AInGameGameMode::NotifyRescueStarted(AEHSurvivorCharacter* Rescuer)
{
	UE_LOG(LogTemp, Log, TEXT("[InGameGM] Rescue started by %s"), *Rescuer->GetName());
}

void AInGameGameMode::NotifyRescueCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("[InGameGM] Rescue completed"));
	EvaluateAllPlayerStates();
}

/* -------------------- PlayerState 평가 -------------------- */
void AInGameGameMode::EvaluateAllPlayerStates()
{
	CheckVictoryConditions();
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, TEXT("EvaluateAllPlayerStates()"));
}

/* -------------------- 승패 판정 -------------------- */
void AInGameGameMode::CheckVictoryConditions()
{
	AInGameGameState* GS = GetGameState<AInGameGameState>();
	if (!GS || GS->bGameEnded) return;

	int32 AliveOrInjured = 0;
	int32 Escaped = 0;
	int32 Captured = 0;

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (AInGamePlayerState* Player = Cast<AInGamePlayerState>(PS))
		{
			if (Player->bIsKiller)
				continue;

			switch (Player->SurvivorState)
			{
			case ESurvivorState::Alive:
			case ESurvivorState::Injured:
				AliveOrInjured++;
				break;

			case ESurvivorState::Captured:
			case ESurvivorState::Dead:
				Captured++;
				break;

			case ESurvivorState::Escaped:
				Escaped++;
				break;
			}
		}
	}

	// 🔹 탈출 조건
	if (Escaped > 0 && AliveOrInjured == 0)
	{
		EndGame(TEXT("Survivors escaped successfully!"));
		return;
	}

	// 🔹 발전기 1개 작동 완료 시
	if (GeneratorCount <= 0)
	{
		EndGame(TEXT("Generator is Repaired!"));
	}
	
	// 🔹 킬러 승리 조건 (모두 포획 or 사망)
	if (AliveOrInjured == 0 && Captured > 0)
	{
		EndGame(TEXT("Killer wins! All survivors captured!"));
	}
}


/* -------------------- EndGame -------------------- */
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

/* -------------------- 발전기 -------------------- */
void AInGameGameMode::CompletedRepair()
{
	if (!HasAuthority()) return;

	GeneratorCount--;
	UE_LOG(LogTemp, Log, TEXT("[GM] GeneratorCount decreased: %d"), GeneratorCount);

	AInGameGameState* GS = GetGameState<AInGameGameState>();
	if (GS)
	{
		// ✅ GameState에 정확한 수치로 설정
		const int32 RepairedCount = 5 - FMath::Max(0, GeneratorCount);
		GS->SetGeneratorsRepaired(RepairedCount);
	}

	if (GeneratorCount <= 0)
	{
		for (TActorIterator<AMultiKeypad> It(GetWorld()); It; ++It)
		{
			if (AMultiKeypad* Keypad = *It)
				Keypad->SetCanInteract(true);
		}
		UE_LOG(LogTemp, Log, TEXT("[GM] All generators repaired. Keypads unlocked."));
	}
}


/* -------------------- 감옥 이동 -------------------- */
void AInGameGameMode::SendToPrison(AInGamePlayerState* CapturedPlayer)
{
	if (!HasAuthority() || !CapturedPlayer) return;

	APlayerController* PC = Cast<APlayerController>(CapturedPlayer->GetOwner());
	if (!PC) return;

	AEHSurvivorCharacter* Survivor = Cast<AEHSurvivorCharacter>(PC->GetPawn());
	if (!Survivor || !PrisonManager) return;

	// ✅ 수감은 반드시 PrisonManager가 수행 (배열 갱신 + 상태/이동 포함)
	PrisonManager->Capture(Survivor);

	EvaluateAllPlayerStates();
}