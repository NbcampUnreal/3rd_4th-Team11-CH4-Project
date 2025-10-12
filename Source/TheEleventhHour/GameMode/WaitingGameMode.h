#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WaitingGameMode.generated.h"

class APlayerStart;

UCLASS()
class THEELEVENTHHOUR_API AWaitingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AWaitingGameMode();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Game")
	FString NextLevelName = TEXT("/Game/Levels/Maps/IngameLevel");

protected:
	/** WaitingLevel에 배치된 PlayerStart 위치를 저장 */
	UPROPERTY()
	TArray<APlayerStart*> SpawnPoints;

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

public:
	UFUNCTION(BlueprintCallable)
	void StartGame();
};
