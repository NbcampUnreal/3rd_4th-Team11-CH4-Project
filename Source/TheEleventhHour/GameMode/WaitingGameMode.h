#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WaitingGameMode.generated.h"

UCLASS()
class THEELEVENTHHOUR_API AWaitingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	UFUNCTION(BlueprintCallable)
	void StartGame();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameFlow")
	FString NextLevelName = "TestMap"; // 넘어갈 맵 이름
};
