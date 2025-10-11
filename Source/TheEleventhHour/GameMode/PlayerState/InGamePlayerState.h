#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "InGamePlayerState.generated.h"

/**
 * Waiting + InGame 공통 PlayerState
 * - 대기실에서 Ready 상태 관리
 * - 게임 중 Killer / Survivor 역할 관리
 * - Survivor 상태 (Alive / Injured / Captured / Dead / Escaped)
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

	/** ✅ 생존자 상태 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_SurvivorState, Category="Game")
	ESurvivorState SurvivorState;

public:
	// ---- 서버 RPC ----

	/** 🔹 서버에 Ready 상태 요청 */
	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);

	/** 🔹 서버에서 생존자 상태 변경 */
	UFUNCTION(Server, Reliable)
	void ServerSetSurvivorState(ESurvivorState NewState);

protected:
	// ---- OnRep ----

	/** Ready 값이 바뀌면 GameState 갱신 */
	UFUNCTION()
	void OnRep_bIsReady();

	/** Killer 변경 로그 출력 */
	UFUNCTION()
	void OnRep_bIsKiller();

	/** 생존 상태 변경 로그 출력 */
	UFUNCTION()
	void OnRep_SurvivorState();

	/** GameState 갱신 */
	void UpdateGameStateReadyCount();

	// ---- 기본 ----
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
