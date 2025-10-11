#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InGameGameMode.generated.h"

class AInGameGameState;
class AInGamePlayerState;

/**
 * InGame 전용 GameMode
 * - 모든 플레이어 로딩 확인 후 역할(킬러/생존자) 분배
 * - 5초 카운트다운 이후 킬러 Pawn 교체
 * - 11분 타이머 및 승패 판정 관리
 */
UCLASS()
class THEELEVENTHHOUR_API AInGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AInGameGameMode();

	virtual void PostSeamlessTravel() override;
	virtual void Tick(float DeltaSeconds) override;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnAllPlayersLoaded();

	/** ✅ Pawn 클래스 (Survivor / Killer) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pawn")
	TSubclassOf<APawn> SurvivorPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pawn")
	TSubclassOf<APawn> KillerPawnClass;

	/** ✅ 내부 상태 관리 */
	bool bMatchStarted = false;
	int32 CountdownTime = 5;
	FTimerHandle CountdownTimer;
	FTimerHandle MatchTimerHandle;

	/** 🔹 11분 게임 제한시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameRule")
	int32 TotalMatchTime = 660;

	/** 🔹 Tick 간격 (1초마다 타이머 감소) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameRule")
	float MatchTickInterval = 1.0f;

	// ---- 역할 / 스폰 관련 ----
	void AssignRoles();
	void StartCountdown();
	void CountdownTick();
	void ReplaceKillerPawn();
	void TryStartMatchWhenAllLoaded();
	
	// ---- 게임 진행 ----
	void StartMatchTimer();
	void MatchTick();
	void CheckVictoryConditions();
	void EndGame(const FString& Reason);
};
