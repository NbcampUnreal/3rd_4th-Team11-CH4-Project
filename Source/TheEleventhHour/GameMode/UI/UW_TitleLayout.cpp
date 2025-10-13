#include "GameMode/UI/UW_TitleLayout.h"

/*#include "Components/Button.h"
#include "GameMode/Controller/TitlePlayerController.h"

// TitleLayout.cpp (UUserWidget)
void UUW_TitleLayout::NativeConstruct()
{
	Super::NativeConstruct();

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &UUW_TitleLayout::OnHostClicked);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UUW_TitleLayout::OnJoinClicked);
	}
}

void UUW_TitleLayout::OnHostClicked()
{
	if (ATitlePlayerController* PC = GetOwningPlayer<ATitlePlayerController>())
	{
		// PC->HostGame(FName("WaitingLevel"), 4, true); // 맵 이름, 최대 인원, LAN 여부
	}
}

void UUW_TitleLayout::OnJoinClicked()
{
	if (ATitlePlayerController* PC = GetOwningPlayer<ATitlePlayerController>())
	{
		// 🔹 세션 검색 후 자동으로 첫 번째 세션 참가
		// PC->FindAndJoinGame();
	}
}

*/