//#pragma once
//
//#include "CoreMinimal.h"
//#include "Blueprint/UserWidget.h"
//#include "TitleUserWidget.generated.h"
//
//class UButton;
//class UEditableTextBox;
//
///**
// * 메인 타이틀 화면에서 서버 접속을 시도하는 UI
// */
//UCLASS()
//class THEELEVENTHHOUR_API UTitleUserWidget : public UUserWidget
//{
//	GENERATED_BODY()
//
//protected:
//	virtual void NativeConstruct() override;
//
//	/** 서버 IP 입력 필드 */
//	UPROPERTY(meta = (BindWidget))
//	UEditableTextBox* ServerIPEditBox;
//
//	/** IP 접속 버튼 */
//	UPROPERTY(meta = (BindWidget))
//	UButton* JoinButton;
//
//	/** 세션 검색 후 자동 참가 버튼 (원한다면) */
//	UPROPERTY(meta = (BindWidgetOptional))
//	UButton* AutoMatchButton;
//
//	UFUNCTION()
//	void OnJoinClicked();
//
//	UFUNCTION()
//	void OnAutoMatchClicked();
//};
