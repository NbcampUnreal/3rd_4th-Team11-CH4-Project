#include "GameMode/PlayerState/InGamePlayerState.h"
#include "GameMode/GameState/WaitingGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

AInGamePlayerState::AInGamePlayerState()
{
	bIsReady = false;
	bIsKiller = false;
	SurvivorState = ESurvivorState::Alive;

	bAlwaysRelevant = true;
}

void AInGamePlayerState::ServerSetReady_Implementation(bool bNewReady)
{
	bIsReady = bNewReady;

	UE_LOG(LogTemp, Log, TEXT("[PS] %s Ready=%d"), *GetPlayerName(), bIsReady);

	UpdateGameStateReadyCount(); // 서버가 GameState 업데이트
}

void AInGamePlayerState::OnRep_bIsReady()
{
	UE_LOG(LogTemp, Log, TEXT("[PS] OnRep_bIsReady Called : %d"), bIsReady);
	UpdateGameStateReadyCount(); // 클라이언트 UI 즉시 반영
}

void AInGamePlayerState::OnRep_bIsKiller()
{
	UE_LOG(LogTemp, Log, TEXT("[PS] %s OnRep_bIsKiller -> %s"),
		*GetPlayerName(),
		bIsKiller ? TEXT("KILLER") : TEXT("SURVIVOR"));
}

void AInGamePlayerState::ServerSetSurvivorState_Implementation(ESurvivorState NewState)
{
	SurvivorState = NewState;

	UE_LOG(LogTemp, Warning, TEXT("[PS] %s SurvivorState -> %s"),
		*GetPlayerName(),
		*UEnum::GetValueAsString(NewState));
}

void AInGamePlayerState::OnRep_SurvivorState()
{
	UE_LOG(LogTemp, Log, TEXT("[PS] %s OnRep_SurvivorState -> %s"),
		*GetPlayerName(),
		*UEnum::GetValueAsString(SurvivorState));
}

void AInGamePlayerState::UpdateGameStateReadyCount()
{
	if (HasAuthority())
	{
		if (AWaitingGameState* GS = GetWorld() ? GetWorld()->GetGameState<AWaitingGameState>() : nullptr)
		{
			GS->UpdateReadyState(); // ReadyCount / TotalPlayers 갱신
		}
	}
	else
	{
		// 클라이언트는 서버가 브로드캐스트하는 GameState로 UI 반영
	}
}

void AInGamePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInGamePlayerState, bIsReady);
	DOREPLIFETIME(AInGamePlayerState, bIsKiller);
	DOREPLIFETIME(AInGamePlayerState, SurvivorState);
}
