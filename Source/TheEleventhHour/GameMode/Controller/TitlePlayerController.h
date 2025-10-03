#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TitlePlayerController.generated.h"

class UTitleUserWidget;

UCLASS()
class THEELEVENTHHOUR_API ATitlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(Client, Reliable)
	void Client_ShowTitleUI();

protected:
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UTitleUserWidget> TitleUserWidgetClass;

private:
	UPROPERTY()
	UTitleUserWidget* TitleUserWidget;
};