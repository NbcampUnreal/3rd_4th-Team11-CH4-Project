#include "PlayerResultEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "GameMode/PlayerState/InGamePlayerState.h"

void UPlayerResultEntryWidget::SetPlayerData(AInGamePlayerState* PlayerState)
{
    if (!PlayerState) return;

    // 플레이어 이름을 설정합니다.
    TextBlock_PlayerName->SetText(FText::FromString(PlayerState->GetPlayerName()));

    // 역할에 따라 다른 통계를 표시합니다.
    if (PlayerState->bIsKiller)
    {
        // 살인마의 통계를 표시합니다.
        TextBlock_NumCaptured->SetText(FText::AsNumber(PlayerState->NumCaptured));
        TextBlock_NumKilled->SetText(FText::AsNumber(PlayerState->NumKilled));
		// 생존자 전용 통계는 "-"로 표시합니다.
        TextBlock_NumGeneratorsRepaired->SetText(FText::FromString(TEXT("-")));
        TextBlock_NumChestsBroken->SetText(FText::FromString(TEXT("-")));
        TextBlock_NumPrisonsUnlocked->SetText(FText::FromString(TEXT("-")));
        TextBlock_NumCapturedTimes->SetText(FText::FromString(TEXT("-")));
    }
    else
    {
        // 살인마 전용 통계는 "-"로 표시합니다.
        TextBlock_NumCaptured->SetText(FText::AsNumber(PlayerState->NumCaptured));
        TextBlock_NumKilled->SetText(FText::AsNumber(PlayerState->NumKilled));

        // 생존자의 통계를 표시합니다.
        TextBlock_NumGeneratorsRepaired->SetText(FText::AsNumber(PlayerState->NumGeneratorsRepaired));
        TextBlock_NumChestsBroken->SetText(FText::AsNumber(PlayerState->NumChestsBroken));
        TextBlock_NumPrisonsUnlocked->SetText(FText::AsNumber(PlayerState->NumPrisonsUnlocked));
        TextBlock_NumCapturedTimes->SetText(FText::AsNumber(PlayerState->NumCapturedTimes));
    }

}