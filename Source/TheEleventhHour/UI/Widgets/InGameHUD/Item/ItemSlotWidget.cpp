#include "ItemSlotWidget.h"
#include "Components/Image.h"

void UItemSlotWidget::UpdateSlot(EItemType ItemType)
{
    // 아이템 타입이 'None'(비어있음)이면 아이콘을 숨깁니다.
    if (ItemType == EItemType::None)
    {
        SetVisibility(ESlateVisibility::Hidden);
        return;
    }

    SetVisibility(ESlateVisibility::Visible);

    // 맵에서 해당 아이템 타입에 맞는 텍스처를 찾아 아이콘을 설정
    if (Image_ItemIcon && ItemTextures.Contains(ItemType))
    {
        Image_ItemIcon->SetBrushFromTexture(ItemTextures[ItemType]);
    }
}