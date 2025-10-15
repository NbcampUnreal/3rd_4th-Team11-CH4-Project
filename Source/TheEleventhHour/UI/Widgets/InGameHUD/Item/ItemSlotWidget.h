#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Item/ItemTypes/ItemTypes.h"
#include "ItemSlotWidget.generated.h"

class UImage;

UCLASS()
class THEELEVENTHHOUR_API UItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateSlot(EItemType ItemType);

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> Image_ItemIcon;

    // 에디터에서 아이템 종류별 텍스처를 지정할 맵
    UPROPERTY(EditAnywhere, Category = "UI")
    TMap<EItemType, TObjectPtr<UTexture2D>> ItemTextures;

};
