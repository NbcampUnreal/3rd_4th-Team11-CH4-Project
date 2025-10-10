// InventoryComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/ItemTypes/ItemTypes.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemAdded, EItemType, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUsed, EItemType, Item, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class THEELEVENTHHOUR_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	UFUNCTION(BlueprintCallable)
	bool AddItem(EItemType Item);              // 서버에서만 성공

	UFUNCTION(BlueprintCallable)
	bool UseItemBySlot(int32 SlotIndex);       // 0 또는 1
	
	UFUNCTION(BlueprintCallable)
	bool ConsumeKey();                         // 서버에서 호출
	
	// UI Getter
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool HasKey() const { return bHasKey; }

	UFUNCTION(BlueprintCallable, Category="Inventory")
	EItemType GetConsumableInSlot(int32 SlotIndex) const {
		return ConsumableSlots.IsValidIndex(SlotIndex) ? ConsumableSlots[SlotIndex] : EItemType::None;
	}
	// UI 델리게이트 제공
	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnItemUsed OnItemUsed;

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_ConsumableSlots();

	UFUNCTION()
	void OnRep_HasKey();

private:
	UPROPERTY(Replicated)
	bool bHasKey = false;                      // 열쇠 전용(최대 1)

	UPROPERTY(Replicated)
	TArray<EItemType> ConsumableSlots;         // 2칸 (Heal/Speed 혼합 가능)
};
