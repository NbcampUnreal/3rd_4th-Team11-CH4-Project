#include "ItemSlotWidget.h"
#include "Components/Image.h"

void UItemSlotWidget::UpdateSlot(EItemType ItemType)
{
    if (GEngine) {
        if (!ItemTextures.IsEmpty()) {
        //    GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, TEXT("ItemTextures TMap is valid and not empty."));
        }
        else {
            //GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("ItemTextures TMap is empty or not linked!"));
        }
    }

    // 아이콘이 유효한지 먼저 확인
    if (!Image_ItemIcon) return;

    if (GEngine)
    {
        //GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Magenta, FString::Printf(TEXT("CLIENT: UpdateSlot() -> ItemType received: %s"), *UEnum::GetValueAsString(ItemType)));
    }

    if (ItemType == EItemType::None)
    {
        Image_ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    Image_ItemIcon->SetVisibility(ESlateVisibility::Visible);

    const TObjectPtr<UTexture2D>* FoundTexture = ItemTextures.Find(ItemType);

    if (FoundTexture && *FoundTexture)
    {
        if (GEngine)
        {
            //GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, TEXT("CLIENT: Texture found and set!"));
        }

        Image_ItemIcon->SetBrushFromTexture(*FoundTexture);
    }
    else
    {
        if (GEngine)
        {
            //GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("CLIENT: Texture NOT found in TMap!"));
        }

        Image_ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
        UE_LOG(LogTemp, Warning, TEXT("ItemSlotWidget: Texture for ItemType '%s' not found in TMap!"), *UEnum::GetValueAsString(ItemType));
    }
}