#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerState.h"
#include "InGamePlayerState.generated.h"

/**
 * Waiting + InGame 통합 PlayerState
 * - 대기실: Ready 상태
 * - 인게임: Killer / Survivor 상태 및 생존 상태 관리
 * - 통계: 역할별 플레이 성과 추적
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

	/* -------------------- ✅ 대기실 관련 -------------------- */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_bIsReady, Category="Lobby")
	bool bIsReady;

	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);

protected:
	UFUNCTION()
	void OnRep_bIsReady();

	void UpdateGameStateReadyCount(); // Waiting 전용


	/* -------------------- ✅ 인게임 관련 -------------------- */
public:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_bIsKiller, Category="Game|Role")
	bool bIsKiller;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_SurvivorState, Category="Game|Survivor")
	ESurvivorState SurvivorState;

	UFUNCTION(Server, Reliable)
	void ServerSetSurvivorState(ESurvivorState NewState);

protected:
	UFUNCTION()
	void OnRep_bIsKiller();

	UFUNCTION()
	void OnRep_SurvivorState();

	void NotifyGameModeStateChanged(); // InGame 전용


	/* -------------------- ✅ 통계 관련 -------------------- */
public:

	// 🎯 [공통] — 모든 플레이어에게 통계 저장
	UPROPERTY(BlueprintReadOnly, Replicated, Category="Stats|Common")
	int32 TotalPlayTime = 0; // 총 플레이 시간 (초)

	// 🔪 [킬러 전용]
	UPROPERTY(BlueprintReadOnly, Replicated, Category="Stats|Killer")
	int32 NumCaptured = 0; // 감옥에 보낸 횟수

	UPROPERTY(BlueprintReadOnly, Replicated, Category="Stats|Killer")
	int32 NumKilled = 0; // 플레이어를 죽인 횟수

	// 🛠️ [생존자 전용]
	UPROPERTY(BlueprintReadOnly, Replicated, Category="Stats|Survivor")
	int32 NumGeneratorsRepaired = 0; // 발전기 수리 횟수

	UPROPERTY(BlueprintReadOnly, Replicated, Category="Stats|Survivor")
	int32 NumChestsBroken = 0; // 상자 부순 횟수

	UPROPERTY(BlueprintReadOnly, Replicated, Category="Stats|Survivor")
	int32 NumPrisonsUnlocked = 0; // 감옥 문 연 횟수

	UPROPERTY(BlueprintReadOnly, Replicated, Category="Stats|Survivor")
	int32 NumCapturedTimes = 0; // 감옥에 갇힌 횟수

	UFUNCTION()
	void AddSurvivorCaptured();


	// ✅ 편의 함수
	void AddGeneratorRepair();
	void AddChestBroken();
	void AddPrisonUnlocked();
	void AddCapturedTime();

	void AddKillerCapture();
	void AddKillerKill();


protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
