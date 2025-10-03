#include "GameMode/UI/WaitingLobbyWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "GameMode/WaitingGameMode.h"
#include "GameMode/GameState/WaitingGameState.h"
#include "GameMode/PlayerState/InGamePlayerState.h"

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

    // ✅ GameState의 델리게이트에 바인딩 → ReadyCount / TotalPlayers가 변경될 때 UI 자동 갱신
    if (AWaitingGameState* GS = GetWorld()->GetGameState<AWaitingGameState>())
    {
        GS->OnPlayerCountUpdated.AddDynamic(this, &UWaitingLobbyWidget::RefreshPlayerCount);
    }
}

void UWaitingLobbyWidget::NativeDestruct()
{
    Super::NativeDestruct();

    // 🔥 위젯이 제거될 때 델리게이트 등록 해제
    if (AWaitingGameState* GS = GetWorld()->GetGameState<AWaitingGameState>())
    {
        GS->OnPlayerCountUpdated.RemoveDynamic(this, &UWaitingLobbyWidget::RefreshPlayerCount);
    }
}

void UWaitingLobbyWidget::UpdateUIForRole()
{
    bool bIsServer = GetOwningPlayer() && GetOwningPlayer()->HasAuthority();

    // 🔹 서버는 Ready 버튼 숨기고 Start 버튼만 표시
    if (ReadyButton)
        ReadyButton->SetVisibility(bIsServer ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

    // 🔹 클라이언트는 Start 버튼 숨김
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
    if (AInGamePlayerState* PS = GetOwningPlayerState<AInGamePlayerState>())
    {
        PS->ServerSetReady(!PS->bIsReady); // 서버로 RPC 전송 → 서버가 Ready 상태 업데이트 및 브로드캐스트
    }
}

void UWaitingLobbyWidget::OnStartClicked()
{
    if (AWaitingGameMode* GM = GetWorld()->GetAuthGameMode<AWaitingGameMode>())
    {
        GM->StartGame(); // 서버가 수동으로 강제 시작
    }
}
