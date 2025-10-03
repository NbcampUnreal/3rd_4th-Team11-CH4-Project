#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WaitingGameMode.generated.h"

UCLASS()
class THEELEVENTHHOUR_API AWaitingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AWaitingGameMode();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Game")
	FString NextLevelName = TEXT("/Game/Levels/Maps/IngameLevel");

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	UFUNCTION(BlueprintCallable)
	void StartGame();
};