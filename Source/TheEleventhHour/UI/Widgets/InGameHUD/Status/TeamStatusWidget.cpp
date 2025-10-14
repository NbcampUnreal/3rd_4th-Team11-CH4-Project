#include "TeamStatusWidget.h"
#include "UI/Widgets/InGameHUD/Status/PlayerStatusWidget.h"

void UTeamStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // 배열에 위젯들을 담아 쉽게 접근할 수 있도록 합니다.
    PlayerIconWidgets.Add(PlayerIcon_0);
    PlayerIconWidgets.Add(PlayerIcon_1);
    PlayerIconWidgets.Add(PlayerIcon_2);
    PlayerIconWidgets.Add(PlayerIcon_3);
    PlayerIconWidgets.Add(PlayerIcon_4);
}

void UTeamStatusWidget::UpdateTeammates(const TArray<AInGamePlayerState*>& PlayerStates)
{
    // 5개의 아이콘 슬롯을 모두 순회합니다.
    for (int32 i = 0; i < PlayerIconWidgets.Num(); ++i)
    {
        // i번째 플레이어가 존재하는지 확인
        if (PlayerStates.IsValidIndex(i) && PlayerStates[i])
        {
            // 플레이어가 존재하면, 아이콘을 보이게 하고 상태에 따라 아이콘을 업데이트합니다.
            AInGamePlayerState* CurrentPS = PlayerStates[i];
            PlayerIconWidgets[i]->SetVisibility(ESlateVisibility::Visible);

            // (PlayerStatusIconWidget에 UpdateState(ESurvivorState State) 함수가 있다고 가정)
            PlayerIconWidgets[i]->UpdateState(CurrentPS->SurvivorState, CurrentPS->GetPlayerName());
        }
        else
        {
            // i번째 플레이어가 존재하지 않으면, 아이콘 자리를 비워놓습니다 (숨김).
            PlayerIconWidgets[i]->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}