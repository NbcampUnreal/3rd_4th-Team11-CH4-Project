#include "GameMode/PlayerState/WaitingPlayerState.h"
#include "GameMode/GameState/WaitingGameState.h"
#include "Net/UnrealNetwork.h"

void AWaitingPlayerState::ServerSetReady_Implementation(bool bNewReady)
{
	bIsReady = bNewReady;

	if (AWaitingGameState* GS = GetWorld()->GetGameState<AWaitingGameState>())
	{
		GS->UpdateReadyState();
	}
}

void AWaitingPlayerState::OnRep_Ready()
{
	if (AWaitingGameState* GS = GetWorld()->GetGameState<AWaitingGameState>())
	{
		GS->UpdateReadyState(); // 클라 UI 즉시 갱신
	}
}

void AWaitingPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWaitingPlayerState, bIsReady);
}