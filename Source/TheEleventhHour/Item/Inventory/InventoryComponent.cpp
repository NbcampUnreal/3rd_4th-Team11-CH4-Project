// InventoryComponent.cpp

#include "Item/Inventory/InventoryComponent.h"
#include "Item/ItemLog/ItemLog.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Slots.Init(EItemType::None, 2);
	UE_LOG(LogItem, Log, TEXT("[Item] LogItem 테스트 출력"));
}

bool UInventoryComponent::AddItem(EItemType Item)
{
	if (Item == EItemType::None) return false;

	for (int32 i=0; i<Slots.Num(); i++)
	{
		if (Slots[i] == EItemType::None)
		{
			Slots[i] = Item;
			UE_LOG(LogItem, Log, TEXT("[Inventory] 슬롯%d에 %s 획득"),
				i, *UEnum::GetValueAsString(Item));
			return true;
		}
	}

	UE_LOG(LogItem, Warning, TEXT("[Inventory] 가득 차서 %s 획득 실패"),
		*UEnum::GetValueAsString(Item));
	return false;
}

bool UInventoryComponent::UseItemBySlot(int32 SlotIndex)
{
	if (!Slots.IsValidIndex(SlotIndex) || Slots[SlotIndex] == EItemType::None)
	{
		UE_LOG(LogItem, Warning, TEXT("[Inventory] 슬롯%d 비어 있음"), SlotIndex);
		return false;
	}

	const EItemType Item = Slots[SlotIndex];
	UE_LOG(LogItem, Log, TEXT("[Use] 슬롯%d의 %s 사용"),
		SlotIndex, *UEnum::GetValueAsString(Item));
	
	Slots[SlotIndex] = EItemType::None;
	return true;
}
