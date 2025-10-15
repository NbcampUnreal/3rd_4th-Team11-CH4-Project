#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Item/ItemTypes/ItemTypes.h"
#include "ItemWidget.generated.h"

class UItemSlotWidget;
class UInventoryComponent;
class UImage;
class UTexture2D;
class UTextBlock;

UCLASS()
class THEELEVENTHHOUR_API UItemWidget : public UUserWidget
{
	GENERATED_BODY()

public: 
	void InitializeInventory();
	
	virtual void NativeConstruct() override;

protected:
	UFUNCTION()
	void OnInventoryUpdated();

	void UpdateKeyUI(bool bHasKey);

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UItemSlotWidget> ItemSlot_0;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UItemSlotWidget> ItemSlot_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_KeyIcon;

	// --- 키 아이콘용 텍스처 2종(에디터에서 세팅) ---
	UPROPERTY(EditAnywhere, Category = "UI|Key")
	TObjectPtr<UTexture2D> KeyIcon_Empty;

	UPROPERTY(EditAnywhere, Category = "UI|Key")
	TObjectPtr<UTexture2D> KeyIcon_Owned;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Text_KeyStatus;

	// 간단 문구
	UPROPERTY(EditAnywhere, Category = "UI|Key")
	FText KeyText_Owned = FText::FromString(TEXT("Owned"));

	UPROPERTY(EditAnywhere, Category = "UI|Key")
	FText KeyText_Empty = FText::FromString(TEXT("Empty"));

	// ✅ 색상도 취향대로
	UPROPERTY(EditAnywhere, Category = "UI|Key")
	FLinearColor KeyTextColor_Owned = FLinearColor(1.0f, 0.f, 0.f, 1.f); 

	UPROPERTY(EditAnywhere, Category = "UI|Key")
	FLinearColor KeyTextColor_Empty = FLinearColor(1.0f, 1.0f, 1.0f, 1.f); // 회색

	UPROPERTY()
	TObjectPtr<UInventoryComponent> LinkedInventory;
};