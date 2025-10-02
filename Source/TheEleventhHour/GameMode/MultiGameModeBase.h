#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MultiGameModeBase.generated.h"

class AEHSurvivorController;

UCLASS()
class THEELEVENTHHOUR_API AMultiGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMultiGameModeBase();
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	virtual void Logout(AController* Exiting) override;

	void CompletedRepair();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<AEHSurvivorController>> AlivePlayerControllers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<AEHSurvivorController>> DeadPlayerControllers;	

	int32 GeneratorCount;
};
