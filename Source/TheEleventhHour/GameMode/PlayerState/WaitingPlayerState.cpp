#include "GameMode/PlayerState/WaitingPlayerState.h"
#include "GameMode/GameState/WaitingGameState.h"
#include "Net/UnrealNetwork.h"

void AWaitingPlayerState::ServerSetReady_Implementation(bool bReady)
{
	bIsReady = bReady;

	if (AWaitingGameState* GS = GetWorld()->GetGameState<AWaitingGameState>())
	{
		GS->UpdateReadyState();       // 서버에서 ReadyCount 갱신
		GS->OnPlayerCountUpdated.Broadcast(); // 🔥 서버가 직접 UI 갱신 알림도 즉시 쏴줌
	}
}

bool AWaitingPlayerState::ServerSetReady_Validate(bool bReady)
{
	return true; // 기본 검증 통과
}

void AWaitingPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWaitingPlayerState, bIsReady);
}