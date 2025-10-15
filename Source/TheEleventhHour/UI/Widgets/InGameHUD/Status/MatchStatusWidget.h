#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchStatusWidget.generated.h"

class UTextBlock;
class UImage;

UCLASS()
class THEELEVENTHHOUR_API UMatchStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateGeneratorCount(int32 Count);
	void ShowExitGateStatus(bool bIsGateOpened);
	
protected:
    UPROPERTY(meta = (BindWidget)) 
    TObjectPtr<UTextBlock> TextBlock_GeneratorCount;

    UPROPERTY(meta = (BindWidget)) 
    TObjectPtr<UImage> Image_GeneratorIcon;

    UPROPERTY(meta = (BindWidget)) 
    TObjectPtr<UImage> Image_ExitGateIcon;

    // 에디터에서 열린/닫힌 탈출구 아이콘 텍스처를 미리 할당
    UPROPERTY(EditAnywhere, Category = "UI") TObjectPtr<UTexture2D> ExitGateOpenedTexture;
    UPROPERTY(EditAnywhere, Category = "UI") TObjectPtr<UTexture2D> ExitGateClosedTexture;
};
