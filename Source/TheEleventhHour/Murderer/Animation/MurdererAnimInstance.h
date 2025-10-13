// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MurdererAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class THEELEVENTHHOUR_API UMurdererAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void AnimNotify_NotifyEvent_FootstepMurderer();

	virtual void NativeInitializeAnimation() override;
protected:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Cached|Character")
	TObjectPtr<class AMurdererCharacter> CachedCharacter;
};
