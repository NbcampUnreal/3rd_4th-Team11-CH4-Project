// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SlateEnums.h"
#include "IPInputPopupWidget.generated.h"

class UEditableTextBox;
class UButton;

UCLASS()
class THEELEVENTHHOUR_API UIPInputPopupWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEditableTextBox> IP_InputBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Apply;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Cancel;

private:
    UFUNCTION()
    void OnApplyClicked();

    UFUNCTION()
    void OnCancelClicked();
};
