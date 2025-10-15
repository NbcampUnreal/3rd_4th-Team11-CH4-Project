#include "ItemWidget.h"
#include "Survivor/Character/EHSurvivorCharacter.h" 
#include "Item/Inventory/InventoryComponent.h"
#include "UI/Widgets/InGameHUD/Item/ItemSlotWidget.h" 
#include "TimerManager.h"

void UItemWidget::NativeConstruct()
{
    Super::NativeConstruct();

    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UItemWidget::InitializeInventory);
}

void UItemWidget::InitializeInventory()
{
    APawn* OwnerPawn = GetOwningPlayerPawn();
    if (AEHSurvivorCharacter* Survivor = Cast<AEHSurvivorCharacter>(OwnerPawn))
    {
        LinkedInventory = Survivor->FindComponentByClass<UInventoryComponent>();
        if (LinkedInventory)
        {
            // InventoryComponent의 OnInventoryUpdated 신호가 오면 OnInventoryUpdated 함수를 호출하도록 '구독'
            LinkedInventory->OnInventoryUpdated.AddDynamic(this, &UItemWidget::OnInventoryUpdated);

            // 현재 인벤토리 상태를 즉시 한번 반영
            OnInventoryUpdated();
        }
    }
}

void UItemWidget::OnInventoryUpdated()
{
    if (!LinkedInventory) return;

    EItemType Item0 = LinkedInventory->GetConsumableInSlot(0);
    EItemType Item1 = LinkedInventory->GetConsumableInSlot(1);

    if (ItemSlot_0) { ItemSlot_0->UpdateSlot(Item0); }
    if (ItemSlot_1) { ItemSlot_1->UpdateSlot(Item1); }
}