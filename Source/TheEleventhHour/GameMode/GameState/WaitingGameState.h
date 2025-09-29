#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "WaitingGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerCountUpdated);

UCLASS()
class THEELEVENTHHOUR_API AWaitingGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AWaitingGameState();
	
	/** 현재 준비 완료한 인원 수 */
	UPROPERTY(ReplicatedUsing=OnRep_ReadyCount, BlueprintReadOnly, Category="Lobby")
	int32 ReadyCount = 0;

	/** 총 플레이어 수 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Lobby")
	int32 TotalPlayers = 0;

	/** UI 에게 알림용 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnPlayerCountUpdated OnPlayerCountUpdated;

	/** 서버에서 호출 → 현재 상태 다시 집계 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void UpdateReadyState();

protected:
	UFUNCTION()
	void OnRep_ReadyCount();

	UFUNCTION()
    void OnRep_TotalPlayers();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
