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

    TArray<AInGamePlayerState*> SurvivorPlayerStates;

    for (AInGamePlayerState* PS : PlayerStates)
    {
        // PlayerState가 유효하고, 'bIsKiller' 변수가 false인 (즉, 생존자인) 경우에만
        if (PS && !PS->bIsKiller)
        {
            // 생존자 목록에 추가합니다.
            SurvivorPlayerStates.Add(PS);
        }
    }

    // 5개의 아이콘 슬롯을 모두 순회합니다.
    for (int32 i = 0; i < PlayerIconWidgets.Num(); ++i)
    {
        // 전체 PlayerStates가 아닌, '생존자만 필터링된' SurvivorPlayerStates 목록을 확인합니다.
        if (SurvivorPlayerStates.IsValidIndex(i))
        {
            // i번째 생존자가 존재하면, 아이콘을 보이게 하고 해당 생존자의 상태로 업데이트합니다.
            AInGamePlayerState* CurrentSurvivorPS = SurvivorPlayerStates[i];
            if (PlayerIconWidgets.IsValidIndex(i) && PlayerIconWidgets[i])
            {
                PlayerIconWidgets[i]->SetVisibility(ESlateVisibility::Visible);
                PlayerIconWidgets[i]->UpdateState(CurrentSurvivorPS->SurvivorState, CurrentSurvivorPS->GetPlayerName());
            }
        }
        else
        {
            if (PlayerIconWidgets.IsValidIndex(i) && PlayerIconWidgets[i])
            {
                PlayerIconWidgets[i]->SetVisibility(ESlateVisibility::Hidden);
            }
        }
    }
}