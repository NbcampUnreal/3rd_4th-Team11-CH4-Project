#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EHAnimInstanceBase.generated.h"

class UCharacterMovementComponent;
class AEHSurvivorCharacter;

UCLASS()
class THEELEVENTHHOUR_API UEHAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 bIsDead : 1;

protected:
	UPROPERTY()
	TObjectPtr<AEHSurvivorCharacter> OwnerCharacter;
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> OwnerCharacterMovementComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Velocity;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float GroundSpeed;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 bShouldMove : 1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 bIsFalling : 1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 bIsCrouching : 1;
};
