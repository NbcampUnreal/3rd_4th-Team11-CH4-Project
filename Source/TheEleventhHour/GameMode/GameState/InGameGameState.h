#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "InGameGameState.generated.h"

/**
 * 인게임 전체 상태를 관리하는 GameState
 * - 남은 시간
 * - 발전기 수리 진행도
 * - 탈출구 오픈 여부
 * - 게임 종료 상태
 * - 로딩 완료 이벤트 브로드캐스트
 */
UCLASS()
class THEELEVENTHHOUR_API AInGameGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AInGameGameState();

	/** ✅ 총 예상 플레이어 수 (서버만 세팅) */
	UPROPERTY(BlueprintReadOnly, Replicated)
	int32 ExpectedPlayers = 0;

	/** ✅ 로딩 완료한 플레이어 수 (서버에서 증가) */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_LoadedPlayers)
	int32 LoadedPlayers = 0;

	/** ✅ 남은 시간 (초 단위) — 11분(660초)에서 시작 */
	UPROPERTY(BlueprintReadOnly, Replicated)
	int32 RemainingTime = 660;

	/** ✅ 발전기 수리 개수 (0 ~ 5) */
	UPROPERTY(BlueprintReadOnly, Replicated)
	int32 GeneratorsRepaired = 0;

	/** ✅ 탈출구 열림 여부 */
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bExitGateOpened = false;

	/** ✅ 게임 종료 여부 */
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bGameEnded = false;

	/** ✅ 게임 종료 사유 */
	UPROPERTY(BlueprintReadOnly, Replicated)
	FString EndReason;

	/** ✅ “모두 로딩 완료” 이벤트 (서버에서만 브로드캐스트됨, GM이 바인딩) */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllPlayersLoaded);
	UPROPERTY(BlueprintAssignable, Category="Match")
	FOnAllPlayersLoaded OnAllPlayersLoaded;

	/* ===================== 서버 전용 호출 함수(클라 RPC 아님) ===================== */

	/** 🔹 서버: 로딩 완료 인원 +1 (PC → 서버RPC에서 호출) */
	UFUNCTION(BlueprintCallable, Category="Match")
	void IncrementLoadedPlayers();

	/** 🔹 서버: 발전기 수리 개수 증가 */
	UFUNCTION(BlueprintCallable)
	void SetGeneratorsRepaired(int32 NewCount);

	/** 🔹 서버: 탈출구 열기 (열리면 남은 시간을 60초로 줄이는 건 GM에서 처리 권장) */
	UFUNCTION(BlueprintCallable, Category="Match")
	void OpenExitGate();

	/** 🔹 서버: 남은 시간 강제 설정 (예: 60초로 세팅) */
	UFUNCTION(BlueprintCallable, Category="Match")
	void SetRemainingTime(int32 NewTime);

	/** 🔹 서버: 게임 종료 세팅 */
	UFUNCTION(BlueprintCallable, Category="Match")
	void EndGame(const FString& Reason);

protected:
	/* ===================== OnRep: 클라 알림 ===================== */

	UFUNCTION()
	void OnRep_LoadedPlayers();

	/* ===================== Net ===================== */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
