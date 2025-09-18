// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/Controller/IntroPlayerController.h"
#include "Blueprint/UserWidget.h"

void AIntroPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = false;
	
	if (IsLocalController())
	{
		if (IntroWidgetClass)
		{
			IntroWidget = CreateWidget<UUserWidget>(this, IntroWidgetClass);
			if (IntroWidget)
			{
				IntroWidget->AddToViewport();
			}
		}
	}
}