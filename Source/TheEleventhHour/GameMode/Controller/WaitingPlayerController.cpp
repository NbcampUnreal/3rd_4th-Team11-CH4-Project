#include "Blueprint/UserWidget.h"
#include "GameMode/UI/WaitingLobbyWidget.h"
#include "GameMode/Controller/WaitingPlayerController.h"

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
