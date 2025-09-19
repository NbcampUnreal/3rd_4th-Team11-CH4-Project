#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GameHUD.generated.h"

UCLASS()
class THEELEVENTHHOUR_API AGameHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	void showMainMenu();
	void showInGameHUD();
	void showResultScreen();

private:
	UUserWidget* CurrentWidget;

	UPROPERTY(EditDefaultsOnly, Category ="UI/Widgets")
	TSubclassOf<UUserWidget> MainMenuClass;
	UPROPERTY(EditDefaultsOnly, Category ="UI/Widgets")
	TSubclassOf<UUserWidget> InGameHUDClass;

	UPROPERTY(EditDefaultsOnly, Category ="UI/Widgets")
	TSubclassOf<UUserWidget> LoadingScreenClass;
	UPROPERTY(EditDefaultsOnly, Category ="UI/Widgets")
	TSubclassOf<UUserWidget> LobbyScreenClass;
	UPROPERTY(EditDefaultsOnly, Category ="UI/Widgets")
	TSubclassOf<UUserWidget> ResultScreenClass;
	
};
