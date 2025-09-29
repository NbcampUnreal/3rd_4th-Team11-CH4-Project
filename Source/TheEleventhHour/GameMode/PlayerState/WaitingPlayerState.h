#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "WaitingPlayerState.generated.h"

UCLASS()
class THEELEVENTHHOUR_API AWaitingPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Lobby")
	bool bIsReady = false;
	

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetReady(bool bReady);
};
