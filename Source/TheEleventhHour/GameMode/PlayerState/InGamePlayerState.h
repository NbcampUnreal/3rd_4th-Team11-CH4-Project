#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerState.h"
#include "InGamePlayerState.generated.h"

/**
 * Waiting + InGame 통합 PlayerState
 * - 대기실: Ready 상태
 * - 인게임: Killer / Survivor 상태 및 생존 상태 관리
 */

UENUM(BlueprintType)
enum class ESurvivorState : uint8
{
	Alive,
	Injured,
	Captured,
	Dead,
	Escaped
};

UCLASS()
class THEELEVENTHHOUR_API AInGamePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AInGamePlayerState();

	/** ✅ 대기실 Ready 여부 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_bIsReady, Category="Lobby")
	bool bIsReady;

	/** ✅ 인게임 Killer 여부 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_bIsKiller, Category="Game")
	bool bIsKiller;

	/** ✅ Survivor 상태 (Alive/Injured/Captured/Dead/Escaped) */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_SurvivorState, Category="Game")
	ESurvivorState SurvivorState;

public:
	// ---- 서버 RPC ----
	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);

	UFUNCTION(Server, Reliable)
	void ServerSetSurvivorState(ESurvivorState NewState);

protected:
	// ---- OnRep ----
	UFUNCTION()
	void OnRep_bIsReady();

	UFUNCTION()
	void OnRep_bIsKiller();

	UFUNCTION()
	void OnRep_SurvivorState();

	// ---- 내부 ----
	void UpdateGameStateReadyCount(); // ✅ Waiting 전용
	void NotifyGameModeStateChanged(); // ✅ InGame 전용

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
