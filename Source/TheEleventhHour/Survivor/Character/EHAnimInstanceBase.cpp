#include "Survivor/Character/EHAnimInstanceBase.h"

#include "Survivor/Character//EHAnimInstanceBase.h"
#include "Survivor/Character/EHSurvivorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UEHAnimInstanceBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<AEHSurvivorCharacter>(GetOwningActor());
	if (IsValid(OwnerCharacter) == true)
	{
		OwnerCharacterMovementComponent = OwnerCharacter->GetCharacterMovement();
	}
}

void UEHAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (IsValid(OwnerCharacter) == false || IsValid(OwnerCharacterMovementComponent) == false)
	{
		return;
	}

	Velocity = OwnerCharacterMovementComponent->Velocity;
	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.f).Size();
	bShouldMove = ((OwnerCharacterMovementComponent->GetCurrentAcceleration().IsNearlyZero()) == false) && (3.f < GroundSpeed);
	bIsFalling = OwnerCharacterMovementComponent->IsFalling();
	bIsCrouching = OwnerCharacterMovementComponent->IsCrouching();
	//bIsAttack = OwnerCharacter->bIsAttack;
	//bIsInteracting = OwnerCharacter->bIsInteracting;
}
