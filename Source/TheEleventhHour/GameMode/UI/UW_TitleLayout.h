#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_TitleLayout.generated.h"

class UButton;

UCLASS()
class THEELEVENTHHOUR_API UUW_TitleLayout : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	/** Host 버튼 (맵을 Listen 서버로 열기) */
	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;

	/** Join 버튼 (추후 세션 참가용으로 확장 가능) */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* JoinButton;

private:
	/** Host 버튼 클릭 시 실행 */
	UFUNCTION()
	void OnHostClicked();

	/** Join 버튼 클릭 시 실행 (원할 경우 구현) */
	UFUNCTION()
	void OnJoinClicked();
};
