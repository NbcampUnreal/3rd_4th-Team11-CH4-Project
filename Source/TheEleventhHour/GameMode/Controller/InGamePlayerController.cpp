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
	}

	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;

	SetInputMode(FInputModeGameOnly());
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
			if (AInGameGameState* GS = GetWorld()->GetGameState<AInGameGameState>())
			{
				TArray<AInGamePlayerState*> AllPlayerStates;
				for (APlayerState* PS : GS->PlayerArray)
				{
					if (AInGamePlayerState* InGamePS = Cast<AInGamePlayerState>(PS))
					{
						AllPlayerStates.Add(InGamePS);
					}
				}
				ResultWidget->PopulateResults(AllPlayerStates);
			}
			ResultWidget->AddToViewport();

			
			UGameplayStatics::SetGamePaused(GetWorld(), true);

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(ResultWidget->TakeWidget());
			SetInputMode(InputMode);
			SetShowMouseCursor(true);
		}
	}
}

#pragma region Spectate System

// 예시 (Controller)
void AInGamePlayerController::ClientEnterSpectatorMode_Implementation()
{
	if (bIsSpectatorMode) return;

	UnPossess(); // Pawn 소유 해제
	ChangeState(NAME_Spectating);

	bIsSpectatorMode = true;

	if (InGameHUDInstance)
	{
		InGameHUDInstance->RemoveFromParent();
		InGameHUDInstance = nullptr;
	}

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	PickInitialSpectateTarget(); // 살아 있는 타겟으로 전환

	//UE_LOG(LogTemp, Warning, TEXT("[InGamePC] 관전자 모드 진입"));
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

void AInGamePlayerController::Client_InitializeRoleHUD_Implementation()
{
	// 이 컨트롤러가 소유한 PlayerState와 HUD 위젯이 모두 유효한지 확인합니다.
	if (InGameHUDInstance)
	{
		if (AInGamePlayerState* InGamePS = GetPlayerState<AInGamePlayerState>())
		{
			// HUD 위젯의 역할 설정 함수를 호출하고, 자신의 역할(bIsKiller)을 전달합니다.
			InGameHUDInstance->SetRoleSpecificVisibility(InGamePS->bIsKiller);
		}
	}
}
