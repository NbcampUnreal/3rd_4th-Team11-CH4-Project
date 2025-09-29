#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UBorder;

UCLASS()
class THEELEVENTHHOUR_API UItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetItemIcon(UTexture2D* NewIcon);
	void SetKeyLabel(const FString& KeyName);
	void SetSelected(bool bSelected);

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* Img_ItemIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_KeyLabel;

	UPROPERTY(meta = (BindWidget))
	UBorder* Br_Highlight;

};
