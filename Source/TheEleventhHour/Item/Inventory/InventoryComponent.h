// InventoryComponent.h

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/ItemTypes/ItemTypes.h"
#include "InventoryComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class THEELEVENTHHOUR_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// 아이템 획득
	UFUNCTION(BlueprintCallable)
	bool AddItem(EItemType Item);

	// 슬롯에서 아이템 사용 (0번 또는 1번)
	UFUNCTION(BlueprintCallable)
	bool UseItemBySlot(int32 SlotIndex);

private:
	UPROPERTY()
	TArray<EItemType> Slots; // 2칸 (None, None)
};
