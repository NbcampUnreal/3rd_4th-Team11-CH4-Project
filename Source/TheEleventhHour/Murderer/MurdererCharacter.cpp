// Fill out your copyright notice in the Description page of Project Settings.


#include "Murderer/MurdererCharacter.h"

// Sets default values
AMurdererCharacter::AMurdererCharacter()
	:BaseSpeed(600.0f)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	
}

// Called when the game starts or when spawned
void AMurdererCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called to bind functionality to input
void AMurdererCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

