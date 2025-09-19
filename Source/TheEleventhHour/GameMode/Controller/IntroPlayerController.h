#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "IntroPlayerController.generated.h"

UCLASS()
class THEELEVENTHHOUR_API AIntroPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> IntroWidgetClass;

private:
	UUserWidget* IntroWidget;
};
