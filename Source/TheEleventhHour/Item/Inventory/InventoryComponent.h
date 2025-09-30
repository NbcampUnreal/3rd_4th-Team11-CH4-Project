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

	UFUNCTION(BlueprintCallable)
	bool AddItem(EItemType Item);              // 서버에서만 성공

	UFUNCTION(BlueprintCallable)
	bool UseItemBySlot(int32 SlotIndex);       // 0 또는 1

	UFUNCTION(BlueprintCallable)
	bool HasKey() const { return bHasKey; }

	UFUNCTION(BlueprintCallable)
	bool ConsumeKey();                         // 서버에서 호출

	UFUNCTION(BlueprintCallable)
	const TArray<EItemType>& GetConsumableSlots() const { return ConsumableSlots; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated)
	bool bHasKey = false;                      // 열쇠 전용(최대 1)

	UPROPERTY(Replicated)
	TArray<EItemType> ConsumableSlots;         // 2칸 (Heal/Speed 혼합 가능)
};
