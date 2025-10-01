#include "GameMode/PlayerState/InGamePlayerState.h"
#include "Net/UnrealNetwork.h"

AInGamePlayerState::AInGamePlayerState()
{
	bIsKiller = false;
}

void AInGamePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AInGamePlayerState, bIsKiller);
}