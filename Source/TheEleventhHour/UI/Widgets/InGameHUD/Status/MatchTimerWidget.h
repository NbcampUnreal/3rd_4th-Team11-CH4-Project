#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchTimerWidget.generated.h"

class UImage;
class UMaterialInstanceDynamic;

UCLASS()
class THEELEVENTHHOUR_API UMatchTimerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateTimer(int32 Time);
	
protected:
	virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> TimerImage;

    // 런타임에 머티리얼의 'Percent' 파라미터를 제어하기 위한 동적 머티리얼 인스턴스
    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> TimerMID;

private:
    // 게임의 전체 시간을 저장할 변수 GameState에서 받아옴.
    float MaxTime = 1.0f; // 0으로 나누는 것 방지
};
