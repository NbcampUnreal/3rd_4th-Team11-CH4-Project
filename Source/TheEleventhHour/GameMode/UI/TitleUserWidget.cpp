#include "GameMode/UI/TitleUserWidget.h"
#include "Components/Button.h"
#include "GameMode/GameInstance/EHGameInstance.h"

void UTitleUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UTitleUserWidget::OnJoinClicked);
	}
}

void UTitleUserWidget::OnJoinClicked()
{
	if (UEHGameInstance* GI = Cast<UEHGameInstance>(GetGameInstance()))
	{
		GI->FindSessionsAndJoin();
	}
}