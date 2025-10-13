#include "GameMode/Controller/InGamePlayerController.h"
#include "GameMode/GameState/InGameGameState.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/GameInstance/EHGameInstance.h"
#include "UI/Widgets/InGameHUD/Core/InGameHUDWidget.h"
#include "UI/Widgets/ResultScreen/ResultScreenWidget.h"

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

		// ✅ 로컬 컨트롤러일 경우, 블루프린트에서 지정한 HUD 위젯을 생성하고 화면에 표시
		if (InGameHUDWidgetClass)
		{
			InGameHUDInstance = CreateWidget<UInGameHUDWidget>(this, InGameHUDWidgetClass);
			if (InGameHUDInstance)
			{
				InGameHUDInstance->AddToViewport();
			}
		}

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
	UE_LOG(LogTemp, Error, TEXT("[ClientUpdateHUDState] : RemainingTime %d GeneratorsRepaired %d bExitOpened %d"), RemainingTime, GeneratorsRepaired, bExitOpened);

	if (InGameHUDInstance)
	{
		InGameHUDInstance->UpdateHUD(RemainingTime, GeneratorsRepaired, bExitOpened);
	}
}

void AInGamePlayerController::ClientOnGameEnded_Implementation(const FString& EndReason)
{
	UE_LOG(LogTemp, Warning, TEXT("[InGamePC] Game Ended: %s"), *EndReason);

	// 기존 인게임 HUD를 화면에서 제거
	if (InGameHUDInstance)
	{
		InGameHUDInstance->RemoveFromParent();
		InGameHUDInstance = nullptr;
	}

	// 결과 화면 위젯을 생성 -> 화면에 표시
	if (ResultScreenWidgetClass)
	{
		UResultScreenWidget* ResultWidget = CreateWidget<UResultScreenWidget>(this, ResultScreenWidgetClass);
		if (ResultWidget)
		{
			// 서버가 보내준 게임 종료 사유(EndReason)를 위젯의 텍스트로 설정.
			ResultWidget->SetResultText(EndReason);
			ResultWidget->AddToViewport();

			// 결과 화면에서는 마우스가 보이고 UI만 조작할 수 있도록 입력 모드를 변경.
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(ResultWidget->TakeWidget());
			SetInputMode(InputMode);
			SetShowMouseCursor(true);
		}
	}
}
