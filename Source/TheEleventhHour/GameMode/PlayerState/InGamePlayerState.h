#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "InGamePlayerState.generated.h"

UCLASS()
class THEELEVENTHHOUR_API AInGamePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AInGamePlayerState();

	/** 킬러인지 여부 (서버 결정 → 클라이언트로 복제) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Role")
	bool bIsKiller;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
