#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InGamePlayerController.generated.h"

class AInGameGameState;
class AInGamePlayerState;
class UInGameHUDWidget;
class UResultScreenWidget;

/**
 * 인게임 플레이어 컨트롤러
 * - 클라이언트가 로딩 완료 시 서버에 알림
 * - 게임 상태 변화를 수신하여 HUD/UI 갱신
 * - 탈출/사망 등 이벤트 처리 예정
 */
UCLASS()
class THEELEVENTHHOUR_API AInGamePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AInGamePlayerController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** ✅ 클라이언트 → 서버 : 로딩 완료 알림 */
	UFUNCTION(Server, Reliable)
	void ServerNotifyClientLoaded();

	/** ✅ 클라이언트 : HUD 상태 갱신용 */
	UFUNCTION(Client, Reliable)
	void ClientUpdateHUDState(int32 RemainingTime, int32 GeneratorsRepaired, bool bExitOpened);

	/** ✅ 클라이언트 : 게임 종료 알림 */
	UFUNCTION(Client, Reliable)
	void ClientOnGameEnded(const FString& EndReason);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Match")
	bool bHasNotifiedLoaded = false;

protected:
	FTimerHandle HUDSyncTimer;

	/** 주기적으로 GameState를 확인하여 HUD 동기화 */
	void SyncHUD();

	/** 내부 참조 캐시 */
	UPROPERTY()
	AInGameGameState* CachedGameState;

	UPROPERTY()
	AInGamePlayerState* CachedPlayerState;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UInGameHUDWidget> InGameHUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UInGameHUDWidget> InGameHUDInstance;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UResultScreenWidget> ResultScreenWidgetClass;

};
