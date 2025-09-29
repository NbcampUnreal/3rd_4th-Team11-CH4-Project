// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WaitingPlayerController.generated.h"

class UWaitingLobbyWidget;

UCLASS()
class THEELEVENTHHOUR_API AWaitingPlayerController : public APlayerController
{
	GENERATED_BODY()

	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UWaitingLobbyWidget> WaitingLayoutClass;

private:
	UPROPERTY()
	UWaitingLobbyWidget* WaitingLayoutWidget;
};
