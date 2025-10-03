#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WaitingLobbyWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * 대기실 UI - Ready / Start 버튼 및 플레이어 상태 표시
 */
UCLASS()
class THEELEVENTHHOUR_API UWaitingLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	/** 준비 버튼 (클라이언트 전용) */
	UPROPERTY(meta = (BindWidget))
	UButton* ReadyButton;

	/** 시작 버튼 (서버 전용) */
	UPROPERTY(meta = (BindWidget))
	UButton* StartButton;

	/** 플레이어 상태 표시용 텍스트 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerCountText;

	/** 준비 상태 토글 */
	UFUNCTION()
	void OnReadyClicked();

	/** 서버가 수동으로 게임 시작 */
	UFUNCTION()
	void OnStartClicked();

	/** Ready / Total 갱신 시 UI 업데이트 */
	UFUNCTION()
	void RefreshPlayerCount();

	/** 현재 유저가 서버인지 클라용인지에 따라 버튼 표시 변경 */
	void UpdateUIForRole();
};
