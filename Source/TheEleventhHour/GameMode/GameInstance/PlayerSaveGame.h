// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PlayerSaveGame.generated.h"

UCLASS()
class THEELEVENTHHOUR_API UPlayerSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPlayerSaveGame();

	UPROPERTY(VisibleAnywhere, Category = "Settings")
	float MasterVolume;

	UPROPERTY(VisibleAnywhere, Category = "Settings")
	float Gamma;

	UPROPERTY(VisibleAnywhere, Category = "Settings")
	float MouseSensitivity;

	UPROPERTY(VisibleAnywhere, Category = "System")
	FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = "System")
	uint32 UserIndex;
};
