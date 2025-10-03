#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "WaitingPlayerState.generated.h"

UCLASS()
class THEELEVENTHHOUR_API AWaitingPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Ready)
	bool bIsReady = false;

	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);

protected:
	UFUNCTION()
	void OnRep_Ready();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};