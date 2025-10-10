#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "InGamePlayerState.generated.h"

/**
 * Waiting + InGame 공통 PlayerState
 * - 대기실에서 Ready 상태 관리
 * - 게임 시작 후 Killer / Survivor 역할 관리
 */
UCLASS()
class THEELEVENTHHOUR_API AInGamePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AInGamePlayerState();

	/** ✅ 대기실 Ready 여부 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_bIsReady)
	bool bIsReady;

	/** ✅ 인게임 Killer 여부 */
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bIsKiller;

	/** 🔹 서버에 Ready 상태 요청 */
	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);

protected:
	/** Ready 값이 바뀌면 GameState 갱신 */
	UFUNCTION()
	void OnRep_bIsReady();

	UFUNCTION()
	void OnRep_bIsKiller();
	
	void UpdateGameStateReadyCount();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
