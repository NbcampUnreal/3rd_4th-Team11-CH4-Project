#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "MatchTimerWidget.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;

UCLASS()
class THEELEVENTHHOUR_API UMatchTimerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateTimer(int32 RemainingTime, bool bExitOpened);
	
protected:
protected:
    virtual void NativeConstruct() override;

    // 위젯들과 연결될 변수들
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage> Image_Background;
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UProgressBar> ProgressBar_Timer;
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_Countdown;

    // 애니메이션과 연결될 변수
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> PulseAnim;

private:
    // 게임의 전체 시간을 저장할 변수 GameState에서 받아옴.
    float MaxTime = 1.0f; // 0으로 나누는 것 방지

};
