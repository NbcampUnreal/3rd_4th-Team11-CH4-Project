#include "GameMode/PlayerState/InGamePlayerState.h"
#include "GameMode/GameState/WaitingGameState.h"
#include "GameMode/InGameGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameMode/Controller/InGamePlayerController.h"
#include "Survivor/Character/EHSurvivorCharacter.h"

AInGamePlayerState::AInGamePlayerState()
{
	bIsReady = false;
	bIsKiller = false;
	SurvivorState = ESurvivorState::Alive;

	bAlwaysRelevant = true;
}

/* -------------------- Ready 상태 -------------------- */
void AInGamePlayerState::ServerSetReady_Implementation(bool bNewReady)
{
	if (bIsReady == bNewReady)
		return;

	bIsReady = bNewReady;
	OnRep_bIsReady();

	UpdateGameStateReadyCount();
}

void AInGamePlayerState::OnRep_bIsReady()
{
	//UE_LOG(LogTemp, Log, TEXT("[PS] %s Ready -> %d"), *GetPlayerName(), bIsReady);
	UpdateGameStateReadyCount();
}

void AInGamePlayerState::UpdateGameStateReadyCount()
{
	if (!GetWorld() || !HasAuthority()) return;

	if (AWaitingGameState* GS = GetWorld()->GetGameState<AWaitingGameState>())
	{
		GS->UpdateReadyState();
	}
}

/* -------------------- Killer -------------------- */
void AInGamePlayerState::OnRep_bIsKiller()
{
	//UE_LOG(LogTemp, Log, TEXT("[PS] %s Role -> %s"),*GetPlayerName(),bIsKiller ? TEXT("KILLER") : TEXT("SURVIVOR"));
}

/* -------------------- Survivor 상태 -------------------- */
void AInGamePlayerState::ServerSetSurvivorState_Implementation(ESurvivorState NewState)
{
	if (SurvivorState == NewState)
		return;

	SurvivorState = NewState;
	OnRep_SurvivorState();
	NotifyGameModeStateChanged();

	// ✅ 감옥에 갇혔을 때 횟수 누적
	if (HasAuthority() && NewState == ESurvivorState::Captured)
	{
		if (AInGameGameMode* GM = GetWorld()->GetAuthGameMode<AInGameGameMode>())
		{
			GM->SendToPrison(this);
			//UE_LOG(LogTemp, Warning, TEXT("[PS] %s has been captured → SendToPrison() called."), *GetPlayerName());
		}
	}
}

void AInGamePlayerState::OnRep_SurvivorState()
{
	const FString StateName = StaticEnum<ESurvivorState>()->GetNameStringByValue((int64)SurvivorState);
	//UE_LOG(LogTemp, Warning, TEXT("[PS] %s SurvivorState -> %s"), *GetPlayerName(), *StateName);

	// ✅ 상태 변경 후 관전 모드 진입 처리
	APlayerController* OwnerPC = Cast<APlayerController>(GetOwner());
	if (!OwnerPC) return;

	// ✅ 로컬 컨트롤러일 때만 적용 (다른 클라에는 적용 안 됨)
	if (OwnerPC->IsLocalController())
	{
		// Dead 또는 Escaped 상태라면 관전 모드 진입
		if (SurvivorState == ESurvivorState::Dead || SurvivorState == ESurvivorState::Escaped)
		{
			if (AInGamePlayerController* InGamePC = Cast<AInGamePlayerController>(OwnerPC))
			{
				//UE_LOG(LogTemp, Warning, TEXT("[PS] %s -> Spectator Mode 진입"), *GetPlayerName());
				InGamePC->ClientEnterSpectatorMode();
			}
		}
	}
}

void AInGamePlayerState::NotifyGameModeStateChanged()
{
	if (HasAuthority())
	{
		if (AInGameGameMode* GM = GetWorld()->GetAuthGameMode<AInGameGameMode>())
		{
			GM->EvaluateAllPlayerStates();
			GM->CheckVictoryConditions();
		}
	}
}

/* -------------------- 통계 관리 -------------------- */
void AInGamePlayerState::AddGeneratorRepair()
{
	if (HasAuthority())
	{
		NumGeneratorsRepaired++;
		//UE_LOG(LogTemp, Log, TEXT("[Stats] %s repaired generator (%d)"), *GetPlayerName(), NumGeneratorsRepaired);
	}
}

void AInGamePlayerState::AddChestBroken()
{
	if (HasAuthority())
	{
		NumChestsBroken++;
	}
}

void AInGamePlayerState::AddPrisonUnlocked()
{
	if (HasAuthority())
	{
		NumPrisonsUnlocked++;
	}
}

void AInGamePlayerState::AddSurvivorCaptured()
{
	if (!HasAuthority()) return;
	
	//UE_LOG(LogTemp, Log, TEXT("[PS] %s captured (%d/3)"), *GetPlayerName(), NumCapturedTimes);

	if (NumCapturedTimes >= 1)
	{
		SurvivorState = ESurvivorState::Dead;

		if (AInGameGameMode* GM = GetWorld()->GetAuthGameMode<AInGameGameMode>())
		{
			if (AEHSurvivorCharacter* Victim = Cast<AEHSurvivorCharacter>(GetPawn()))
			{
				GM->OnSurvivorKilled(Victim); // ✅ 사망 처리
			}
		}
	}
}

void AInGamePlayerState::AddCapturedTime()
{
	if (HasAuthority())
	{
		NumCapturedTimes++;
		//UE_LOG(LogTemp, Log, TEXT("[Stats] %s captured times: %d"), *GetPlayerName(), NumCapturedTimes);
	}
}

void AInGamePlayerState::AddKillerCapture()
{
	if (HasAuthority())
	{
		NumCaptured++;
	}
}

void AInGamePlayerState::AddKillerKill()
{
	if (HasAuthority())
	{
		NumKilled++;
	}
}

/* -------------------- Replication -------------------- */
void AInGamePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInGamePlayerState, bIsReady);
	DOREPLIFETIME(AInGamePlayerState, bIsKiller);
	DOREPLIFETIME(AInGamePlayerState, SurvivorState);

	// 🔹 통계 데이터 복제
	DOREPLIFETIME(AInGamePlayerState, TotalPlayTime);
	
	DOREPLIFETIME(AInGamePlayerState, NumCaptured);
	DOREPLIFETIME(AInGamePlayerState, NumKilled);
	
	DOREPLIFETIME(AInGamePlayerState, NumGeneratorsRepaired);
	DOREPLIFETIME(AInGamePlayerState, NumChestsBroken);
	DOREPLIFETIME(AInGamePlayerState, NumPrisonsUnlocked);
	DOREPLIFETIME(AInGamePlayerState, NumCapturedTimes);
}
