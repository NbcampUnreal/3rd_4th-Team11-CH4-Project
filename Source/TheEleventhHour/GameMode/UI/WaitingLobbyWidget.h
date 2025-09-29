#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WaitingLobbyWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class THEELEVENTHHOUR_API UWaitingLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	/** Ready 버튼 (클라이언트 전용) */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* ReadyButton;

	/** Start 버튼 (호스트 전용) */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* StartButton;

	/** 준비 인원 표시용 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* PlayerCountText;

private:
	UFUNCTION()
	void OnReadyClicked();

	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void RefreshPlayerCount();
	
	void UpdateUIForRole();
};
