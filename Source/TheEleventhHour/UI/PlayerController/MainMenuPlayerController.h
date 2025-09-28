#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuPlayerController.generated.h"

class UUserWidget;
class UMediaPlayer;
class UMediaSource;

UCLASS()
class THEELEVENTHHOUR_API AMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI/Widgets");
	TSubclassOf<class UUserWidget> MainMenuWidgetClass;

	UUserWidget* MainMenuInstance;

public:
	UFUNCTION(BlueprintCallable, Category = "UI/Events")
	void HandlePlaySelected();

	UFUNCTION()
	void OnZoomInEnd();

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI/Media", meta = (AllowPrivateAccess="true"))
	UMediaPlayer* MP_ZoomIn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI/Media", meta = (AllowPrivateAccess = "true"))
	UMediaSource* MS_ZoomIn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI/Media", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> ZoomInVideoWidgetClass;

	UPROPERTY()
	UUserWidget* ZoomInVideoInstance;
};
