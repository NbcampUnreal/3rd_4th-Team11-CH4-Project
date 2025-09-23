// InventoryComponent.cpp

#include "Item/Inventory/InventoryComponent.h"
#include "Item/ItemLog/ItemLog.h"
#include "Survivor/Character/AttributeComponent.h"
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

	// ===== 효과 적용 시작 =====
	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		if (UAttributeComponent* Attr = OwnerActor->FindComponentByClass<UAttributeComponent>())
		{
			switch (Item)
			{
			case EItemType::Heal:
				{
					const float HealAmount = 50.f; // 테스트 값
					const float NewHP = FMath::Clamp(Attr->GetHealth() + HealAmount, 0.f, Attr->MaxHealth); // B안(직접 참조)
					Attr->SetHealth(NewHP);
					UE_LOG(LogItem, Log, TEXT("[Use] Heal 사용: +%.0f (이제 %.0f/%.0f)"),
						HealAmount, NewHP, Attr->MaxHealth);
					break;
				}
			case EItemType::Speed:
				{
					const float AddSpeed = 200.f;   // 테스트 값
					const float Duration = 5.f;     // 테스트 값(초)
					Attr->ApplySpeedBuff(AddSpeed, Duration); // ↓ 2)에서 추가할 헬퍼
					UE_LOG(LogItem, Log, TEXT("[Use] Speed 사용: +%.0f, 지속 %.1fs"),
						AddSpeed, Duration);
					break;
				}
			default:
				break;
			}
		}
	}
	// ===== 효과 적용 끝 =====

	UE_LOG(LogItem, Log, TEXT("[Use] 슬롯%d의 %s 사용"),
		SlotIndex, *UEnum::GetValueAsString(Item));

	Slots[SlotIndex] = EItemType::None; // 사용 후 비우기
	return true;
}
