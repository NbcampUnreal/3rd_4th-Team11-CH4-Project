#include "MainMenuPlayerController.h"
#include "UI/Widgets/MainMenu/MainMenuWidget.h"

#include "Blueprint/UserWidget.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "Kismet/GameplayStatics.h"

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (MP_ZoomIn)
	{
		MP_ZoomIn->OnEndReached.AddDynamic(this, &AMainMenuPlayerController::OnZoomInEnd);
	}

	if (MainMenuWidgetClass)
	{
		MainMenuInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);

		if (MainMenuInstance)
		{
			MainMenuInstance->AddToViewport();

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(MainMenuInstance->TakeWidget());
			SetInputMode(InputMode);
			bShowMouseCursor = false; // Keyboard Only

			if (UMainMenuWidget* Menu = Cast<UMainMenuWidget>(MainMenuInstance))
			{
				Menu->OnPlaySelected.AddDynamic(this, &AMainMenuPlayerController::HandlePlaySelected);
			}
		}
	}
}

void AMainMenuPlayerController::HandlePlaySelected()
{
	if (ZoomInVideoWidgetClass)
	{
		ZoomInVideoInstance = CreateWidget<UUserWidget>(this, ZoomInVideoWidgetClass);
		
		if (ZoomInVideoInstance)
		{
			ZoomInVideoInstance->AddToViewport(10);
		}
	}

	if (MP_ZoomIn && MS_ZoomIn)
	{
		MP_ZoomIn->OpenSource(MS_ZoomIn); // Play Vedio
	}
}

void AMainMenuPlayerController::OnZoomInEnd()
{
	if (MainMenuInstance)
	{
		MainMenuInstance->RemoveFromParent();
		MainMenuInstance = nullptr;
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;

	UGameplayStatics::OpenLevel(GetWorld(), TEXT("TestMap"), true);
}
