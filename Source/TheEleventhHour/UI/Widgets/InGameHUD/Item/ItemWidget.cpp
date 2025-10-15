#include "ItemWidget.h" 
#include "Item/Inventory/InventoryComponent.h"
#include "Survivor/Character/EHSurvivorCharacter.h"
#include "UI/Widgets/InGameHUD/Item/ItemSlotWidget.h"
#include "Components/Image.h" 
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "GameMode/PlayerState/InGamePlayerState.h"

void UItemWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UItemWidget::InitializeInventory()
{
    if (GEngine)
    {
        //GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Cyan, TEXT("CLIENT: OnRep_ConsumableSlots() called. Broadcasting UI update..."));
    }

    if (APawn* OwnerPawn = GetOwningPlayerPawn())
    {
        if (AEHSurvivorCharacter* Survivor = Cast<AEHSurvivorCharacter>(OwnerPawn))
        {
            LinkedInventory = Survivor->FindComponentByClass<UInventoryComponent>();
            if (LinkedInventory)
            {
                //GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, TEXT("CLIENT: InventoryComponent successfully linked!"));

                LinkedInventory->OnInventoryUpdated.AddDynamic(this, &UItemWidget::OnInventoryUpdated);

                OnInventoryUpdated();
            }
        }
    }

    //GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("CLIENT: Failed to link InventoryComponent. UI won't update."));
}

void UItemWidget::OnInventoryUpdated()
{
    if (!LinkedInventory) return;

    if (GEngine)
    {
        //GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Blue, TEXT("CLIENT: UItemWidget::OnInventoryUpdated() called. Updating item slots."));
    }

    EItemType Item0 = LinkedInventory->GetConsumableInSlot(0);
    EItemType Item1 = LinkedInventory->GetConsumableInSlot(1);

    if (ItemSlot_0) ItemSlot_0->UpdateSlot(Item0);
    if (ItemSlot_1) ItemSlot_1->UpdateSlot(Item1);

    UpdateKeyUI(LinkedInventory->HasKey());
}

void UItemWidget::UpdateKeyUI(bool bHasKey)
{
    // ---- 아이콘 처리 ----
    if (Image_KeyIcon)
    {
        UTexture2D* TargetTex = bHasKey ? KeyIcon_Owned : KeyIcon_Empty;

        if (TargetTex)
        {
            Image_KeyIcon->SetBrushFromTexture(TargetTex);
            Image_KeyIcon->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            Image_KeyIcon->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    if (Text_KeyStatus)
    {
        const FText  TargetText = bHasKey ? KeyText_Owned : KeyText_Empty;
        const FLinearColor TargetColor = bHasKey ? KeyTextColor_Owned : KeyTextColor_Empty;

        Text_KeyStatus->SetText(TargetText);
        Text_KeyStatus->SetColorAndOpacity(TargetColor);
        Text_KeyStatus->SetVisibility(ESlateVisibility::Visible);
    }
}