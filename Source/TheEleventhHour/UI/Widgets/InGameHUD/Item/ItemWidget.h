#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemWidget.generated.h"

class UItemSlotWidget;
class UInventoryComponent;

UCLASS()
class THEELEVENTHHOUR_API UItemWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
    virtual void NativeConstruct() override;

    void InitializeInventory();

    UPROPERTY(meta = (BindWidget)) TObjectPtr<UItemSlotWidget> ItemSlot_0;
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UItemSlotWidget> ItemSlot_1;

    // 인벤토리 컴포넌트의 신호를 받을 함수
    UFUNCTION()
    void OnInventoryUpdated();

    // 인벤토리 컴포넌트의 참조를 저장할 변수
    UPROPERTY()
    TObjectPtr<UInventoryComponent> LinkedInventory;

};
