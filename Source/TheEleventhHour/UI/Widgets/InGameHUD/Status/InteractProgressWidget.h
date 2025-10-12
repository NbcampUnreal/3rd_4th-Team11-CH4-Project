#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractProgressWidget.generated.h"

class UProgressBar;

UCLASS()
class THEELEVENTHHOUR_API UInteractProgressWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void SetProgress(float InProgress);

protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ProgressBar_Repair;
};
