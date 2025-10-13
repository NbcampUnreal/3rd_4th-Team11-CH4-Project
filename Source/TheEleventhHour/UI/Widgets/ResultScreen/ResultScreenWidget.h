#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResultScreenWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class THEELEVENTHHOUR_API UResultScreenWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    // PlayerController가 호출할 함수. 결과 텍스트를 설정.
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetResultText(const FString& InResultText);

protected:
    virtual void NativeConstruct() override;

    // UMG 디자이너의 위젯들과 연결될 변수
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_ResultTitle;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_BackToMenu;

private:
    // '메인 메뉴로 돌아가기' 버튼을 클릭했을 때 호출될 함수
    UFUNCTION()
    void OnBackToMenuClicked();
};
