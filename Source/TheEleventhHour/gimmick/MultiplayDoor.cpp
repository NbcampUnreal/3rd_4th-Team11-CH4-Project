// Fill out your copyright notice in the Description page of Project Settings.


#include "gimmick/MultiplayDoor.h"

// Sets default values
AMultiplayDoor::AMultiplayDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMultiplayDoor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMultiplayDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

