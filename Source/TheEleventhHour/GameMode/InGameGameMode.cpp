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
#include "GameMode/Controller/InGamePlayerController.h"
#include "UI/Widgets/InGameHUD/Core/InGameHUDWidget.h"

AInGameGameMode::AInGameGameMode()
	: PrisonManager(nullptr)
{
	bUseSeamlessTravel = true;

	GameStateClass = AInGameGameState::StaticClass();
	PlayerStateClass = AInGamePlayerState::StaticClass();

	// 나중에 BeginPlay에서 세팅할 예정
	RemainingGeneratorCount = TotalGeneratorCount;
}


/* -------------------- BeginPlay -------------------- */
void AInGameGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		RemainingGeneratorCount = TotalGeneratorCount;

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
	static TSet<APlayerStart*> UsedStarts; // 이미 사용된 스타트 기록 (게임 세션 동안 유지)

	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPoints);

	TArray<APlayerStart*> PlayerTaggedStarts;

	// 🔹 "Spawn.Player" 태그가 붙은 스타트만 필터링
	for (AActor* Actor : SpawnPoints)
	{
		if (APlayerStart* Start = Cast<APlayerStart>(Actor))
		{
			if (Start->PlayerStartTag == FName("Spawn.Player"))
			{
				// ✅ 아직 사용되지 않은 Start만 추가
				if (!UsedStarts.Contains(Start))
				{
					PlayerTaggedStarts.Add(Start);
				}
			}
		}
	}

	// ✅ 사용 가능한 스타트가 있다면 랜덤 선택
	if (PlayerTaggedStarts.Num() > 0)
	{
		const int32 Index = FMath::RandRange(0, PlayerTaggedStarts.Num() - 1);
		APlayerStart* ChosenStart = PlayerTaggedStarts[Index];

		UsedStarts.Add(ChosenStart); // 사용 처리
		UE_LOG(LogTemp, Log, TEXT("[ChoosePlayerStart] Spawned at %s"), *ChosenStart->GetName());

		return ChosenStart;
	}

	// 🚨 모든 Start가 이미 사용되었거나 없을 경우 — 마지막 선택 사용
	UE_LOG(LogTemp, Warning, TEXT("[ChoosePlayerStart] All 'Spawn.Player' points used or missing! Using default."));
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

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			// PlayerController를 InGamePlayerController 타입으로 변환(Cast)합니다.
			if (AInGamePlayerController* PC = Cast<AInGamePlayerController>(It->Get()))
			{
				// 해당 컨트롤러의 Client_InitializeRoleHUD 함수를 호출하여 신호를 보냅니다.
				PC->Client_InitializeRoleHUD();
			}
		}

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

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AInGamePlayerController* PC = Cast<AInGamePlayerController>(It->Get());
		if (PC)
		{
			PC->ClientOnGameEnded(Reason); // 🎯 여기서 호출!
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[InGameGM] GAME OVER: %s"), *Reason);
}

/* -------------------- 발전기 -------------------- */
void AInGameGameMode::CompletedRepair()
{
	if (!HasAuthority()) return;

	RemainingGeneratorCount = FMath::Clamp(RemainingGeneratorCount - 1, 0, TotalGeneratorCount);
	UE_LOG(LogTemp, Log, TEXT("[GM] Generator repaired → Remaining: %d / %d"), 
		   RemainingGeneratorCount, TotalGeneratorCount);

	AInGameGameState* GS = GetGameState<AInGameGameState>();
	if (GS)
	{
		// ✅ GameState에 정확한 수치로 전달
		const int32 RepairedCount = TotalGeneratorCount - RemainingGeneratorCount;
		GS->SetGeneratorsRepaired(RepairedCount);
	}

	if (RemainingGeneratorCount <= 0)
	{
		for (TActorIterator<AMultiKeypad> It(GetWorld()); It; ++It)
		{
			if (AMultiKeypad* Keypad = *It)
				Keypad->SetCanInteract(true);
		}
		UE_LOG(LogTemp, Log, TEXT("[GM] ✅ All generators repaired. Keypads unlocked."));
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

	// ✅ 감옥에 실제로 수감 (위치 이동 + 상태 변경)
	PrisonManager->Capture(Survivor);

	// ✅ 감금 횟수 1 증가 (PlayerState 내부에서 3회 시 사망 자동 처리)
	CapturedPlayer->AddSurvivorCaptured();

	// ✅ 디버그 로그
	UE_LOG(LogTemp, Log, TEXT("[InGameGM] %s captured (%d/3)"),
		*CapturedPlayer->GetPlayerName(),
		CapturedPlayer->NumCapturedTimes);

	// ✅ 생존자 상태 평가 (게임 진행 상황 갱신)
	EvaluateAllPlayerStates();
	CheckVictoryConditions();

	LogAllPlayerStates();  // ✅ 추가
}


void AInGameGameMode::OnEscapeDoorOpened()
{
	if (!HasAuthority()) return;

	AInGameGameState* GS = GetGameState<AInGameGameState>();
	if (!GS) return;

	// 🔹 1분 30초로 남은 시간 고정
	GS->RemainingTime = 90; // 초 단위 (1분 30초)

	// 🔹 게이트가 열렸음을 표시
	GS->bExitGateOpened = true;

	UE_LOG(LogTemp, Warning, TEXT("[InGameGM] 🚪 Escape door opened! Remaining time set to 90 seconds."));

	// 🔹 클라이언트 HUD 갱신 등을 위해 브로드캐스트 할 수도 있음 (옵션)
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 3.f, FColor::Green,
			TEXT("Escape Door Opened! 1 minute 30 seconds remaining!")
		);
	}
}

// ✅ Pawn 이동 + 입력 차단 + 관전 진입 (UI 유지)
void AInGameGameMode::HandlePlayerRemoved(AController* Controller, const FString& Reason)
{
	if (!Controller) return;

	APawn* ControlledPawn = Controller->GetPawn();
	if (ControlledPawn)
	{
		Controller->UnPossess();

		TArray<AActor*> SpawnPoints;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPoints);

		for (AActor* Actor : SpawnPoints)
		{
			if (APlayerStart* Start = Cast<APlayerStart>(Actor))
			{
				if (Start->PlayerStartTag == FName("Spawn.End"))
				{
					ControlledPawn->SetActorLocation(Start->GetActorLocation());
					ControlledPawn->SetActorRotation(Start->GetActorRotation());
					break;
				}
			}
		}

		ControlledPawn->DisableInput(nullptr); // 조작 불가 처리
	}

	if (AInGamePlayerController* PC = Cast<AInGamePlayerController>(Controller))
	{
		PC->ClientEnterSpectatorMode();
	}
}

void AInGameGameMode::OnSurvivorKilled(AEHSurvivorCharacter* Victim)
{
	if (!HasAuthority() || !Victim) return;

	AInGamePlayerState* KillerPS = nullptr;
	AInGamePlayerState* VictimPS = Cast<AInGamePlayerState>(Victim->GetPlayerState());

	// 🔹 킬러 찾기 (GameState의 PlayerArray에서 bIsKiller == true인 사람)
	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (AInGamePlayerState* PlayerPS = Cast<AInGamePlayerState>(PS))
		{
			if (PlayerPS->bIsKiller)
			{
				KillerPS = PlayerPS;
				break;
			}
		}
	}

	// 🔹 킬러의 킬 카운트 +1
	if (KillerPS)
	{
		KillerPS->AddKillerKill();
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] %s killed %s"),
			*KillerPS->GetPlayerName(),
			VictimPS ? *VictimPS->GetPlayerName() : TEXT("Unknown Victim"));
	}

	// 🔹 희생자 상태 변경
	if (VictimPS)
	{
		VictimPS->SurvivorState = ESurvivorState::Dead;
		VictimPS->ForceNetUpdate();
	}

	HandlePlayerRemoved(Victim->GetController(), TEXT("You were killed."));
	
	// 🔹 게임 상태 재평가 (승리 조건 확인)
	EvaluateAllPlayerStates();
	CheckVictoryConditions();

	LogAllPlayerStates();  // ✅ 추가
}

void AInGameGameMode::LogAllPlayerStates()
{
	if (!HasAuthority()) return;

	UE_LOG(LogTemp, Warning, TEXT("======== [Player Status Report] ========"));

	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (AInGamePlayerState* PlayerPS = Cast<AInGamePlayerState>(PS))
		{
			const FString RoleName = PlayerPS->bIsKiller ? TEXT("KILLER") : TEXT("SURVIVOR");
			const FString StateName = StaticEnum<ESurvivorState>()->GetNameStringByValue((int64)PlayerPS->SurvivorState);

			if (PlayerPS->bIsKiller)
			{
				UE_LOG(LogTemp, Warning, TEXT(" - %s [%s] | Kills: %d, Captured: %d"),
					*PlayerPS->GetPlayerName(),
					*RoleName,
					PlayerPS->NumKilled,
					PlayerPS->NumCaptured);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT(" - %s [%s] | State: %s | Captured: %d/2 | Repairs: %d | Chests: %d | Prisons: %d"),
					*PlayerPS->GetPlayerName(),
					*RoleName,
					*StateName,
					PlayerPS->NumCapturedTimes,
					PlayerPS->NumGeneratorsRepaired,
					PlayerPS->NumChestsBroken,
					PlayerPS->NumPrisonsUnlocked);
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("========================================"));
}
