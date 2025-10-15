#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OptionsWidget.generated.h"

class UButton;
class UOptionRow;

UCLASS()
class THEELEVENTHHOUR_API UOptionsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UOptionRow> WBP_OptionRow_Gamma;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UOptionRow> WBP_OptionRow_Volume;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UOptionRow> WBP_OptionRow_Sensitivity;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Back;

private:
    void UpdateSelectionVisuals();
    void ApplySettingChange();

    UFUNCTION()
    void OnBackButtonClicked();

    UPROPERTY()
    TArray<TObjectPtr<UOptionRow>> OptionRows;

    UPROPERTY()
    int32 CurrentRowIndex = 0;

    UPROPERTY()
    bool bIsEditingValue = false;

    UPROPERTY()
    TObjectPtr<class UEHGameInstance> GameInstanceRef;
};
