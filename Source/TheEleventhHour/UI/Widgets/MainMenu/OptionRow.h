// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OptionRow.generated.h"

class UTextBlock;
class UHorizontalBox;
class UBorder;

UCLASS()
class THEELEVENTHHOUR_API UOptionRow : public UUserWidget
{
	GENERATED_BODY()

public:
    void UpdateVisuals();
    void ChangeValue(int32 Amount);

    UPROPERTY(BlueprintReadWrite, Category = "Option Row")
    int32 CurrentValue = 0;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_Name;

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> HorizontalBox_Values;

private:
    UPROPERTY()
    TArray<TObjectPtr<UBorder>> ValueBoxes;

};
