//#pragma once
//
//#include "CoreMinimal.h"
//#include "Blueprint/UserWidget.h"
//#include "Animation/WidgetAnimation.h" //
//#include "LoadingScreenWidget.generated.h"
//
//class UProgressBar;
//class UTextBlock;
//
//UCLASS()
//class THEELEVENTHHOUR_API ULoadingScreenWidget : public UUserWidget
//{
//	GENERATED_BODY()
//	
//public:
//    void PlayLoadingAnimation();
//
//protected:
//
//    virtual void NativeConstruct() override;
//
//    // UMG 디자이너의 위젯들과 C++ 변수를 연결합니다.
//    UPROPERTY(meta = (BindWidget))
//    TObjectPtr<UProgressBar> ProgressBar_Loading;
//
//    UPROPERTY(meta = (BindWidget))
//    TObjectPtr<UTextBlock> TextBlock_Percent;
//
//    UPROPERTY(Transient, meta = (BindWidgetAnim))
//    TObjectPtr<UWidgetAnimation> LoadingAnim;
//
//};
