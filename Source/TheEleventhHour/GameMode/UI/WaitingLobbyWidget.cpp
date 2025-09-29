#include "GameMode/UI/WaitingLobbyWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "GameMode/WaitingGameMode.h"
#include "GameMode/GameState/WaitingGameState.h"
#include "GameMode/PlayerState/WaitingPlayerState.h"

void UWaitingLobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ReadyButton)
    {
        ReadyButton->OnClicked.AddDynamic(this, &UWaitingLobbyWidget::OnReadyClicked);
    }

    if (StartButton)
    {
        StartButton->OnClicked.AddDynamic(this, &UWaitingLobbyWidget::OnStartClicked);
    }

    UpdateUIForRole();
    RefreshPlayerCount();

    // GameState 이벤트 바인딩
    if (AWaitingGameState* GS = GetWorld()->GetGameState<AWaitingGameState>())
    {
        GS->OnPlayerCountUpdated.AddDynamic(this, &UWaitingLobbyWidget::RefreshPlayerCount);
    }
}

void UWaitingLobbyWidget::NativeDestruct()
{
    Super::NativeDestruct();

    if (AWaitingGameState* GS = GetWorld()->GetGameState<AWaitingGameState>())
    {
        GS->OnPlayerCountUpdated.RemoveDynamic(this, &UWaitingLobbyWidget::RefreshPlayerCount);
    }
}

void UWaitingLobbyWidget::UpdateUIForRole()
{
    bool bIsServer = GetOwningPlayer() && GetOwningPlayer()->HasAuthority();

    if (ReadyButton)
        ReadyButton->SetVisibility(bIsServer ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

    if (StartButton)
        StartButton->SetVisibility(bIsServer ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UWaitingLobbyWidget::RefreshPlayerCount()
{
    if (PlayerCountText)
    {
        if (AWaitingGameState* GS = GetWorld()->GetGameState<AWaitingGameState>())
        {
            PlayerCountText->SetText(
                FText::FromString(FString::Printf(TEXT("Ready: %d / %d"), GS->ReadyCount, GS->TotalPlayers))
            );
        }
    }
}

void UWaitingLobbyWidget::OnReadyClicked()
{
    if (AWaitingPlayerState* PS = GetOwningPlayerState<AWaitingPlayerState>())
    {
        bool bNewReady = !PS->bIsReady;
        PS->ServerSetReady(bNewReady);

        // 🔥 로컬에서도 즉시 갱신 (서버에서 돌아오기 전 화면 반영)
        RefreshPlayerCount();
    }
}

void UWaitingLobbyWidget::OnStartClicked()
{
    if (GetOwningPlayer() && GetOwningPlayer()->HasAuthority())
    {
        if (AWaitingGameMode* GM = GetWorld()->GetAuthGameMode<AWaitingGameMode>())
        {
            GM->StartGame();

            // 🔥 Start 누르면 ReadyCount가 바로 갱신될 수도 있으니 즉시 반영
            RefreshPlayerCount();
        }
    }
}