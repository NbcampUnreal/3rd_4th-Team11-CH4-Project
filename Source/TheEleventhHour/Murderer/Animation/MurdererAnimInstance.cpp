// Fill out your copyright notice in the Description page of Project Settings.


#include "Murderer/Animation/MurdererAnimInstance.h"

#include "Murderer/MurdererCharacter.h"

void UMurdererAnimInstance::AnimNotify_NotifyEvent_FootstepMurderer()
{
	CachedCharacter->Server_PlayFootstepSound();
}

void UMurdererAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	CachedCharacter=Cast<AMurdererCharacter>(TryGetPawnOwner());
}
