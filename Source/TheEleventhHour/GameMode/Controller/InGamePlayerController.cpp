#include "GameMode/Controller/InGamePlayerController.h"
#include "GameMode/GameState/InGameGameState.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "Survivor/Character/EHSurvivorCharacter.h"
#include "UI/Widgets/InGameHUD/Core/InGameHUDWidget.h"
#include "UI/Widgets/ResultScreen/ResultScreenWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AInGamePlayerController::AInGamePlayerController()
{
	bReplicates = true;
	bHasNotifiedLoaded = false;
	bIsSpectatorMode = false;
}

void AInGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		ServerNotifyClientLoaded();

		if (InGameHUDWidgetClass)
		{
			InGameHUDInstance = CreateWidget<UInGameHUDWidget>(this, InGameHUDWidgetClass);
			if (InGameHUDInstance)
			{
				InGameHUDInstance->AddToViewport();
			}
		}

		GetWorld()->GetTimerManager().SetTimer(
			HUDSyncTimer, this, &AInGamePlayerController::SyncHUD, 1.0f, true, 2.0f);

		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;

		SetInputMode(FInputModeGameOnly());
	}
}

void AInGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 기본적으로 관전 입력은 바인딩 해두되, 생존 상태에선 무시
	InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ThisClass::HandleSpectateNextInput);
	InputComponent->BindKey(EKeys::Q, IE_Pressed, this, &ThisClass::HandleSpectatePrevInput);
}

void AInGamePlayerController::HandleSpectateNextInput()
{
	if (bIsSpectatorMode)
	{
		NextSpectateTarget();
	}
}

void AInGamePlayerController::HandleSpectatePrevInput()
{
	if (bIsSpectatorMode)
	{
		PrevSpectateTarget();
	}
}

void AInGamePlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!CachedGameState)
		CachedGameState = GetWorld()->GetGameState<AInGameGameState>();
	if (!CachedPlayerState)
		CachedPlayerState = GetPlayerState<AInGamePlayerState>();
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
	if (InGameHUDInstance)
	{
		InGameHUDInstance->UpdateHUD(RemainingTime, GeneratorsRepaired, bExitOpened);
	}
}

void AInGamePlayerController::ClientOnGameEnded_Implementation(const FString& EndReason)
{
	if (InGameHUDInstance)
	{
		InGameHUDInstance->RemoveFromParent();
		InGameHUDInstance = nullptr;
	}

	if (ResultScreenWidgetClass)
	{
		UResultScreenWidget* ResultWidget = CreateWidget<UResultScreenWidget>(this, ResultScreenWidgetClass);
		if (ResultWidget)
		{
			ResultWidget->SetResultText(EndReason);
			ResultWidget->AddToViewport();

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(ResultWidget->TakeWidget());
			SetInputMode(InputMode);
			SetShowMouseCursor(true);
		}
	}
}

#pragma region Spectate System

void AInGamePlayerController::ClientEnterSpectatorMode_Implementation()
{
	UnPossess();
	ChangeState(NAME_Spectating);

	bIsSpectatorMode = true;

	bShowMouseCursor = false;
	FInputModeGameOnly Mode;
	SetInputMode(Mode);

	PickInitialSpectateTarget();

	UE_LOG(LogTemp, Warning, TEXT("[InGamePC] Entered Spectator Mode."));
}

void AInGamePlayerController::CollectAliveSpectateCandidates(TArray<AEHSurvivorCharacter*>& Out) const
{
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEHSurvivorCharacter::StaticClass(), Found);

	for (AActor* A : Found)
	{
		if (AEHSurvivorCharacter* Survivor = Cast<AEHSurvivorCharacter>(A))
		{
			if (AInGamePlayerState* PS = Survivor->GetPlayerState<AInGamePlayerState>())
			{
				if (!PS->bIsKiller && (PS->SurvivorState == ESurvivorState::Alive || PS->SurvivorState == ESurvivorState::Injured))
				{
					Out.Add(Survivor);
				}
			}
		}
	}
}

void AInGamePlayerController::PickInitialSpectateTarget()
{
	ServerSetSpectateTarget(true);
}

void AInGamePlayerController::NextSpectateTarget()
{
	ServerSetSpectateTarget(true);
}

void AInGamePlayerController::PrevSpectateTarget()
{
	ServerSetSpectateTarget(false);
}

void AInGamePlayerController::ServerSetSpectateTarget_Implementation(bool bNext)
{
	TArray<AEHSurvivorCharacter*> Candidates;
	CollectAliveSpectateCandidates(Candidates);
	if (Candidates.Num() == 0) return;

	int32 CurIndex = INDEX_NONE;
	for (int32 i = 0; i < Candidates.Num(); ++i)
	{
		if (GetViewTarget() == Candidates[i])
		{
			CurIndex = i;
			break;
		}
	}

	int32 NextIndex = 0;
	if (CurIndex == INDEX_NONE)
		NextIndex = 0;
	else
		NextIndex = bNext ? (CurIndex + 1) % Candidates.Num()
			: (CurIndex - 1 + Candidates.Num()) % Candidates.Num();

	SetViewTargetWithBlend(Candidates[NextIndex], 0.4f, VTBlend_Cubic);

	const FString NameForUI = Candidates[NextIndex]->GetPlayerState()
		? Candidates[NextIndex]->GetPlayerState()->GetPlayerName()
		: Candidates[NextIndex]->GetName();

	ClientUpdateSpectateUI(NameForUI);
}

void AInGamePlayerController::ClientUpdateSpectateUI_Implementation(const FString& TargetPlayerName)
{
	if (InGameHUDInstance)
	{
		// BlueprintImplementableEvent로 HUD에 표시 (옵션)
		// InGameHUDInstance->UpdateSpectateName(TargetPlayerName);
	}
}

#pragma endregion
