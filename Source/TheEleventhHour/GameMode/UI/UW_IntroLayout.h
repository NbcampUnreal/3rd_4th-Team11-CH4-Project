#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_IntroLayout.generated.h"

UCLASS()
class THEELEVENTHHOUR_API UUW_IntroLayout : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* FadeIn;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* FadeOut;
	
	UFUNCTION()
	void OnFadeOutFinished();
	
	void StartFadeOut();
	
	FWidgetAnimationDynamicEvent FadeOutFinishedDelegate;
	
	FTimerHandle FadeOutTimerHandle;
};
