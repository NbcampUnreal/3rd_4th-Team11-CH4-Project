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
    if (Image_ItemIcon)
    {
        // 1. ItemTextures 맵에서 ItemType에 해당하는 텍스처 포인터를 찾습니다.
        const TObjectPtr<UTexture2D>* FoundTexture = ItemTextures.Find(ItemType);

        // 2. 포인터를 성공적으로 찾았고, 그 포인터가 가리키는 텍스처도 유효한지 확인합니다.
        if (FoundTexture && *FoundTexture)
        {
            // 3. 찾은 텍스처로 아이콘 이미지의 브러시를 설정합니다.
            Image_ItemIcon->SetBrushFromTexture(*FoundTexture);
        }
        else
        {
            // 만약 맵에 해당 아이템의 아이콘이 지정되지 않았다면, 안전을 위해 이미지를 비웁니다.
            Image_ItemIcon->SetVisibility(ESlateVisibility::Hidden);
            UE_LOG(LogTemp, Warning, TEXT("ItemSlotWidget: Texture for ItemType '%s' not found in TMap!"), *UEnum::GetValueAsString(ItemType));
        }
    }
}