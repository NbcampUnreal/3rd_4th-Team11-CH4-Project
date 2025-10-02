#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleUserWidget.generated.h"

UCLASS()
class THEELEVENTHHOUR_API UTitleUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	/** Join 버튼 */
	UPROPERTY(meta = (BindWidget))
	class UButton* JoinButton;

	/** 버튼 클릭 이벤트 */
	UFUNCTION()
	void OnJoinClicked();
};
