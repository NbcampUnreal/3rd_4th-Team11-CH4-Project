#include "MainMenuWidget.h"

#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameMode/Controller/TitlePlayerController.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

}

void UMainMenuWidget::InitializeMenu()
{
	this->bIsFocusable = true;

	MenuIndex = { PlayButton, JoinButton, OptionsButton, CreditsButton, ExitButton };
	CurrentIndex = 0;
	bVisibleState = true;

	UpdateSelection();
}

FReply UMainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::Up)
	{
		MoveSelection(-1);
		return FReply::Handled();
	}
	else if (Key == EKeys::Down)
	{
		MoveSelection(1);
		return FReply::Handled();
	}
	else if (Key == EKeys::Enter)
	{
		ConfirmSelection();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UMainMenuWidget::MoveSelection(int32 Direction)
{
	CurrentIndex = (CurrentIndex + Direction + MenuIndex.Num()) % MenuIndex.Num();
	UpdateSelection();
}

void UMainMenuWidget::UpdateSelection()
{
	// All Texts Initialize
	for (UTextBlock* Text : MenuIndex)
	{
		if (Text)
		{
			Text->SetRenderOpacity(1.0f);
			Text->SetColorAndOpacity(FSlateColor(FLinearColor::White)); // Basic Color =  White
		}
	}

	// Clean Up Previous Timer
	GetWorld()->GetTimerManager().ClearTimer(BlinkTimerHandle);

	if (MenuIndex.IsValidIndex(CurrentIndex))
	{
		MenuIndex[CurrentIndex]->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
	}

	GetWorld()->GetTimerManager().SetTimer(
		BlinkTimerHandle,
		this,
		&UMainMenuWidget::BlinkCurrentIndex,
		0.5f, // Opacity Interval Time
		true
	);
}

void UMainMenuWidget::ConfirmSelection()
{
	UE_LOG(LogTemp, Warning, TEXT("MainMenuWidget::ConfirmSelection >> CurrentIndex: %d"), CurrentIndex);

	switch (CurrentIndex)
	{
	case 0: // Play
		UE_LOG(LogTemp, Error, TEXT("MainMenuWidget >> TRY BROADCAST: Play (Index %d)"), CurrentIndex);
		OnHostClicked.Broadcast();
		break;
	case 1: // Join
		OnJoinClicked.Broadcast();
		break;
	case 2: // Options
		UE_LOG(LogTemp, Error, TEXT("MainMenuWidget >> TRY BROADCAST: Options (Index %d)"), CurrentIndex);
		OnOptionsClicked.Broadcast();
		break;
	case 3: // Credits

		break;
	case 4: // Exit
		UE_LOG(LogTemp, Error, TEXT("MainMenuWidget >> TRY BROADCAST: Exit (Index %d)"), CurrentIndex);
		UKismetSystemLibrary::QuitGame(this, GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, true);
		break;
	}
}

void UMainMenuWidget::BlinkCurrentIndex()
{
	if (!MenuIndex.IsValidIndex(CurrentIndex)) return;

	bVisibleState = !bVisibleState;
	float Opacity = bVisibleState ? 1.0f : 0.3f;

	MenuIndex[CurrentIndex]->SetRenderOpacity(Opacity);
}
