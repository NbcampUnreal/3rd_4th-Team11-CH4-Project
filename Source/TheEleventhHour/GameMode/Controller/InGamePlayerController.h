#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InGamePlayerController.generated.h"

class AInGameGameState;
class AInGamePlayerState;
class UInGameHUDWidget;
class UResultScreenWidget;
class AEHSurvivorCharacter;

UCLASS()
class THEELEVENTHHOUR_API AInGamePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AInGamePlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
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

	/** ✅ 관전 모드 진입 */
	UFUNCTION(Client, Reliable)
	void ClientEnterSpectatorMode();

	/** ✅ 관전 UI 갱신 */
	UFUNCTION(Client, Reliable)
	void ClientUpdateSpectateUI(const FString& TargetPlayerName);

protected:
	UPROPERTY()
	AInGameGameState* CachedGameState;

	UPROPERTY()
	AInGamePlayerState* CachedPlayerState;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UInGameHUDWidget> InGameHUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UInGameHUDWidget> InGameHUDInstance;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UResultScreenWidget> ResultScreenWidgetClass;

	UPROPERTY()
	bool bHasNotifiedLoaded;

	UPROPERTY()
	bool bIsSpectatorMode;

	FTimerHandle HUDSyncTimer;

	/** ✅ HUD 주기적 동기화 */
	void SyncHUD();

	/** ======= 관전 시스템 ======= */
	void HandleSpectateNextInput();
	void HandleSpectatePrevInput();

	void CollectAliveSpectateCandidates(TArray<AEHSurvivorCharacter*>& Out) const;
	void PickInitialSpectateTarget();

	UFUNCTION(Server, Reliable)
	void ServerSetSpectateTarget(bool bNext);

	void NextSpectateTarget();
	void PrevSpectateTarget();
};
