#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InGameGameMode.generated.h"

UCLASS()
class THEELEVENTHHOUR_API AInGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AInGameGameMode();
	
protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	void AssignRoles(); // 🔥 Killer 지정

	void CheckAllPlayersLoaded();
	
	UPROPERTY(EditDefaultsOnly, Category="Spawn")
	TSubclassOf<APawn> KillerPawnClass;

	UPROPERTY(EditDefaultsOnly, Category="Spawn")
	TSubclassOf<APawn> SurvivorPawnClass;

	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;

private:
	bool bRolesAssigned;
};
