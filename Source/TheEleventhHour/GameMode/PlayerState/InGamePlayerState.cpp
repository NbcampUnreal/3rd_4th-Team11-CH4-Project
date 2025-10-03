#include "GameMode/PlayerState/InGamePlayerState.h"
#include "GameMode/GameState/WaitingGameState.h"
#include "Net/UnrealNetwork.h"

AInGamePlayerState::AInGamePlayerState()
{
	bIsReady = false; 
	bIsKiller = false;
	bAlwaysRelevant = true;
}

void AInGamePlayerState::ServerSetReady_Implementation(bool bNewReady)
{
	bIsReady = bNewReady;
	UE_LOG(LogTemp, Log, TEXT("[InGamw dePS] %s Ready=%d"), *GetPlayerName(), bIsReady);

	UpdateGameStateReadyCount(); // 서버가 GameState 업데이트
}

void AInGamePlayerState::OnRep_bIsReady()
{
	UE_LOG(LogTemp, Log, TEXT("[InGamePS] OnRep_bIsReady Called : %d"), bIsReady);
	UpdateGameStateReadyCount(); // 클라이언트도 값 바뀌면 UI 즉시 반영
}

void AInGamePlayerState::OnRep_bIsKiller()
{
	UE_LOG(LogTemp, Log, TEXT("[PS] %s OnRep_bIsKiller -> %s"),
		*GetPlayerName(),
		bIsKiller ? TEXT("KILLER") : TEXT("SURVIVOR"));
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
		// 클라에서도 UpdateReadyState()를 호출할지 여부는 선택적.
		// (보통 서버만 호출해도 OnPlayerCountUpdated 델리게이트로 UI가 갱신됩니다)
	}
}

void AInGamePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInGamePlayerState, bIsReady);
	DOREPLIFETIME(AInGamePlayerState, bIsKiller);
}
