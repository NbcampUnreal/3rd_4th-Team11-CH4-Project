#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InGameGameMode.generated.h"

class AInGameGameState;
class AInGamePlayerState;
class AMultiKeypad;
class AEHSurvivorCharacter;
class APrisonManager;

/**
 * InGame 전용 GameMode
 * - Waiting 단계에서 이어받음 (SeamlessTravel)
 * - 모든 플레이어 로딩 확인 후 역할 분배 및 카운트다운
 * - 11분 제한 시간 / 승패 판정 / 발전기-게이트 연결
 * - 감옥 시스템 통합
 */
UCLASS()
class THEELEVENTHHOUR_API AInGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AInGameGameMode();
	// ---- 감옥 관련 ----
	UPROPERTY()
	APrisonManager* PrisonManager;

	/** 🔹 감옥 관련 이벤트 */
	void CapturePlayer(AEHSurvivorCharacter* Victim);
	void NotifyRescueStarted(AEHSurvivorCharacter* Rescuer);
	void NotifyRescueCompleted();

	/** 🔹 PlayerState 변화 평가 */
	UFUNCTION()
	void EvaluateAllPlayerStates();

	/** 🔹 승패 판정 */
	void CheckVictoryConditions();

	/** 🔹 발전기 완성 시 호출 */
	void CompletedRepair();

	/** ✅ 생존자 감옥 이동 (감옥 매니저 통해 처리) */
	UFUNCTION(BlueprintCallable, Category="GameRule")
	void SendToPrison(AInGamePlayerState* CapturedPlayer);

protected:
	virtual void BeginPlay() override;
	virtual void PostSeamlessTravel() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	
	UFUNCTION()
	void OnAllPlayersLoaded();

	// ---- Pawn 클래스 ----
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pawn")
	TSubclassOf<APawn> SurvivorPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pawn")
	TSubclassOf<APawn> KillerPawnClass;

	// ---- 내부 상태 ----
	bool bMatchStarted = false;
	int32 CountdownTime = 5;
	FTimerHandle CountdownTimer;
	FTimerHandle MatchTimerHandle;

	// ---- 게임 규칙 ----
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameRule")
	int32 TotalMatchTime = 660; // 11분

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameRule")
	float MatchTickInterval = 1.0f;

	// ---- 발전기 ----
	int32 GeneratorCount;

	// ---- 진행 ----
	void AssignRoles();
	void StartCountdown();
	void CountdownTick();
	void ReplaceKillerPawn();
	void TryStartMatchWhenAllLoaded();
	void StartMatchTimer();
	void MatchTick();
	void EndGame(const FString& Reason);
};