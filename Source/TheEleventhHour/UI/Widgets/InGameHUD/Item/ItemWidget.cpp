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
    UE_LOG(LogTemp, Warning, TEXT("[ItemWidget] OwnerPawn=%s"),
        OwnerPawn ? *OwnerPawn->GetName() : TEXT("NULL"));

    if (AEHSurvivorCharacter* Survivor = Cast<AEHSurvivorCharacter>(OwnerPawn))
    {
        LinkedInventory = Survivor->FindComponentByClass<UInventoryComponent>();
        UE_LOG(LogTemp, Warning, TEXT("[ItemWidget] LinkedInventory=%s"),
            LinkedInventory ? *LinkedInventory->GetName() : TEXT("NULL"));

        if (LinkedInventory)
        {
            LinkedInventory->OnInventoryUpdated.AddDynamic(this, &UItemWidget::OnInventoryUpdated);
            OnInventoryUpdated();
        }
    }
}

void UItemWidget::OnInventoryUpdated()
{
    UE_LOG(LogTemp, Warning, TEXT("[ItemWidget] OnInventoryUpdated CALLED"));
    if (!LinkedInventory) return;

    EItemType Item0 = LinkedInventory->GetConsumableInSlot(0);
    EItemType Item1 = LinkedInventory->GetConsumableInSlot(1);
    UE_LOG(LogTemp, Warning, TEXT("[ItemWidget] Slots: [0]=%s [1]=%s"),
        *UEnum::GetValueAsString(Item0), *UEnum::GetValueAsString(Item1));

    if (ItemSlot_0) ItemSlot_0->UpdateSlot(Item0);
    if (ItemSlot_1) ItemSlot_1->UpdateSlot(Item1);
}