#include "GameMode/UI/TitleUserWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "GameMode/GameInstance/EHGameInstance.h"

void UTitleUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UTitleUserWidget::OnJoinClicked);
	}
	if (AutoMatchButton)
	{
		AutoMatchButton->OnClicked.AddDynamic(this, &UTitleUserWidget::OnAutoMatchClicked);
	}
}

void UTitleUserWidget::OnJoinClicked()
{
	if (UEHGameInstance* GI = Cast<UEHGameInstance>(GetGameInstance()))
	{
		FString InputIP = TEXT("127.0.0.1");
		if (ServerIPEditBox)
		{
			const FString UserText = ServerIPEditBox->GetText().ToString().TrimStartAndEnd();
			if (!UserText.IsEmpty())
			{
				InputIP = UserText;
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[TitleUI] ConnectToIP: %s"), *InputIP);
		GI->ConnectToIP(InputIP);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[TitleUI] GameInstance cast failed"));
	}
}

void UTitleUserWidget::OnAutoMatchClicked()
{
	if (UEHGameInstance* GI = Cast<UEHGameInstance>(GetGameInstance()))
	{
		UE_LOG(LogTemp, Log, TEXT("[TitleUI] Start FindSessionsAndJoin"));
		GI->FindSessionsAndJoin();
	}
}
