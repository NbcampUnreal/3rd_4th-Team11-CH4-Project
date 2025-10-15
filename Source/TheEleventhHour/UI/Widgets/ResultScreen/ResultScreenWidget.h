#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResultScreenWidget.generated.h"

class UTextBlock;
class UVerticalBox;
class UPlayerResultEntryWidget;
class AInGamePlayerState;
class UButton;

UCLASS()
class THEELEVENTHHOUR_API UResultScreenWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    // PlayerController가 호출하여 결과 화면을 채우는 함수
    void PopulateResults(const TArray<AInGamePlayerState*>& AllPlayerStates);

protected:
    virtual void NativeConstruct() override;

    // UMG 디자이너와 연결될 변수들
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_PlayerMatchResult; // "승리" 또는 "패배"
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UVerticalBox> VerticalBox_KillerTeam; // 살인마 팀 목록
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UVerticalBox> VerticalBox_SurvivorTeam; // 생존자 팀 목록
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_BackToMenu;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UPlayerResultEntryWidget> PlayerResultEntryWidgetClass;

private:
    UFUNCTION() 
    void OnBackToMenuClicked();
};
