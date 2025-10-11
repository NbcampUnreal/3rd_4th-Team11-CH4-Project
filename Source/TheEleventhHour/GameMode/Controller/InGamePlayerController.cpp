#include "GameMode/Controller/InGamePlayerController.h"
#include "GameMode/GameState/InGameGameState.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AInGamePlayerController::AInGamePlayerController()
{
	bReplicates = true;
	bHasNotifiedLoaded = false;
}

void AInGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("[InGamePC] BeginPlay"));

	if (IsLocalController())
	{
		// ✅ 클라이언트가 맵 로딩 완료 시 서버에 알림
		ServerNotifyClientLoaded();

		// ✅ 일정 주기로 HUD 업데이트 (GameState 값 반영)
		GetWorld()->GetTimerManager().SetTimer(
			HUDSyncTimer, this, &AInGamePlayerController::SyncHUD, 1.0f, true, 2.0f);
	}
}

void AInGamePlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 캐시 안되어 있으면 캐싱
	if (!CachedGameState)
	{
		CachedGameState = GetWorld()->GetGameState<AInGameGameState>();
	}
	if (!CachedPlayerState)
	{
		CachedPlayerState = GetPlayerState<AInGamePlayerState>();
	}
}

void AInGamePlayerController::ServerNotifyClientLoaded_Implementation()
{
	if (bHasNotifiedLoaded) return;
	bHasNotifiedLoaded = true;

	if (AInGameGameState* GS = GetWorld()->GetGameState<AInGameGameState>())
	{
		GS->IncrementLoadedPlayers();
	}
}


void AInGamePlayerController::SyncHUD()
{
	if (!CachedGameState)
		CachedGameState = GetWorld()->GetGameState<AInGameGameState>();
	if (!CachedGameState) return;

	ClientUpdateHUDState(
		CachedGameState->RemainingTime,
		CachedGameState->GeneratorsRepaired,
		CachedGameState->bExitGateOpened
	);
}

void AInGamePlayerController::ClientUpdateHUDState_Implementation(int32 RemainingTime, int32 GeneratorsRepaired, bool bExitOpened)
{
	// ✅ HUD 갱신: 여기서는 예시로 로그 출력, 이후 UUserWidget 연결
	FString TimeStr = FString::Printf(TEXT("⏰ %02d:%02d"), RemainingTime / 60, RemainingTime % 60);
	FString GenStr = FString::Printf(TEXT("⚙️ %d / 5 Generators"), GeneratorsRepaired);
	FString ExitStr = bExitOpened ? TEXT("🚪 Exit Gate OPEN") : TEXT("🔒 Exit Gate Locked");

	UE_LOG(LogTemp, Log, TEXT("[HUD] %s | %s | %s"), *TimeStr, *GenStr, *ExitStr);

	// 👉 나중에 HUD Widget을 연결하면 여기서 Widget에 값 전달 가능
	// ex) if (UInGameHUDWidget* HUD = GetHUDWidget()) HUD->UpdateInfo(RemainingTime, GeneratorsRepaired, bExitOpened);
}

void AInGamePlayerController::ClientOnGameEnded_Implementation(const FString& EndReason)
{
	UE_LOG(LogTemp, Warning, TEXT("[InGamePC] Game Ended: %s"), *EndReason);

	// ✅ UI를 통해 GameOver 화면 표시 예정
	// ex) ShowGameOverWidget(EndReason);
}
