#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMenuButtonClicked);

class UTextBlock;

UCLASS()
class THEELEVENTHHOUR_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** Host 버튼 클릭 시 실행 */
	UPROPERTY(BlueprintAssignable, Category = "UI/Events")
	FOnMenuButtonClicked OnHostClicked;
	/** Join 버튼 클릭 시 실행 (원할 경우 구현) */
	UPROPERTY(BlueprintAssignable, Category = "UI/Events")
	FOnMenuButtonClicked OnJoinClicked;
	/** Opions 버튼 클릭 시 실행 */
	UPROPERTY(BlueprintAssignable, Category = "UI/Events")
	FOnMenuButtonClicked OnOptionsClicked;

	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
	// PlayerController가 호출할 공개 초기화 함수
	void InitializeMenu();

private:
	/** Host 버튼 (맵을 Listen 서버로 열기) */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayButton;

	/** Join 버튼 (추후 세션 참가용으로 확장 가능) */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* JoinButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* OptionsButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CreditsButton;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ExitButton;
	
	// Menu Array
	TArray<UTextBlock*> MenuIndex;

	// Current Selected Menu
	int32 CurrentIndex;
	FTimerHandle BlinkTimerHandle;
	bool bVisibleState;

	//Focus Update Function
	void UpdateSelection();

	void MoveSelection(int32 Direction);

	void ConfirmSelection();

	void BlinkCurrentIndex();
};
