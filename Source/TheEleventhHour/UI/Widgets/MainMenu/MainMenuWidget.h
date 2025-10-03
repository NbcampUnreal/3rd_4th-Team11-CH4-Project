#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlaySelected);

UCLASS()
class THEELEVENTHHOUR_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category = "UI/Events")
	FOnPlaySelected OnPlaySelected;

	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayButton;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* OptionsButton;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CreditsButton;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ExitButton;
	
	// Menu Array
	TArray<UTextBlock*> MenuIndex;

	// Current Selected Menu
	int32 CurrentIndex;
	FTimerHandle BlinkTimerHandle;
	bool bVisibleState;

	//Focus Update Function
	void UpdateSelection();

	void MoveSelection(int32 Direction);

	void ConfirmSelection();

	void BlinkCurrentIndex();
};
