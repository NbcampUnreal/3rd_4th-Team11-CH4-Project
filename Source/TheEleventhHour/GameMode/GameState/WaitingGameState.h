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

	/** ✅ 준비한 인원 수 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ReadyCount)
	int32 ReadyCount;

	/** ✅ 총 플레이어 수 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_TotalPlayers)
	int32 TotalPlayers;

	/** 🔹 UI에 브로드캐스트할 이벤트 */
	UPROPERTY(BlueprintAssignable)
	FOnPlayerCountUpdated OnPlayerCountUpdated;

	/** ✅ 서버에서 Ready 상태 갱신 */
	void UpdateReadyState();

protected:
	/** 🔹 RepNotify 콜백 */
	UFUNCTION()
	void OnRep_ReadyCount();

	UFUNCTION()
	void OnRep_TotalPlayers();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
