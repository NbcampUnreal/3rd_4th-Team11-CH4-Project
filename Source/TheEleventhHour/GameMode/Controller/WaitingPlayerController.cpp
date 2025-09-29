#include "GameMode/Controller/WaitingPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "GameMode/UI/WaitingLobbyWidget.h"



void AWaitingPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (WaitingLayoutClass)
	{
		WaitingLayoutWidget = CreateWidget<UWaitingLobbyWidget>(this, WaitingLayoutClass);
		if (WaitingLayoutWidget)
		{
			WaitingLayoutWidget->AddToViewport();
		}
	}
}
