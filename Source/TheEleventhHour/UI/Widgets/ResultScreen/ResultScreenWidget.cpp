#include "ResultScreenWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "PlayerResultEntryWidget.h" 
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "Kismet/GameplayStatics.h"

void UResultScreenWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (Button_BackToMenu)
    {
        Button_BackToMenu->OnClicked.AddDynamic(this, &UResultScreenWidget::OnBackToMenuClicked);
    }
}

void UResultScreenWidget::PopulateResults(const TArray<AInGamePlayerState*>& AllPlayerStates)
{
    // --- 1. 로컬 플레이어의 최종 결과 판정 ---
    AInGamePlayerState* MyPlayerState = GetOwningPlayerState<AInGamePlayerState>();
    if (MyPlayerState && TextBlock_PlayerMatchResult)
    {
        if (MyPlayerState->bIsKiller)
        {
            const int32 Kills = MyPlayerState->NumKilled;
            if (Kills >= 4) 
            { 
                TextBlock_PlayerMatchResult->SetText(FText::FromString(TEXT("Massacre"))); 
                TextBlock_PlayerMatchResult->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
            }
            else if (Kills >= 2) 
            { 
                TextBlock_PlayerMatchResult->SetText(FText::FromString(TEXT("Brutal"))); 
                TextBlock_PlayerMatchResult->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.5f, 0.0f)));
            }
            else 
            { 
                TextBlock_PlayerMatchResult->SetText(FText::FromString(TEXT("Merciful"))); 
                TextBlock_PlayerMatchResult->SetColorAndOpacity(FSlateColor(FLinearColor::White));
            }
        }
        else
        {
            switch (MyPlayerState->SurvivorState)
            {
            case ESurvivorState::Escaped: 
                TextBlock_PlayerMatchResult->SetText(FText::FromString(TEXT("Escaped"))); 
                TextBlock_PlayerMatchResult->SetColorAndOpacity(FSlateColor(FLinearColor::White));
                break;
            case ESurvivorState::Dead: 
                TextBlock_PlayerMatchResult->SetText(FText::FromString(TEXT("Murdered"))); 
                TextBlock_PlayerMatchResult->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
                break;
            case ESurvivorState::Captured:
                TextBlock_PlayerMatchResult->SetText(FText::FromString(TEXT("Murdered")));
                TextBlock_PlayerMatchResult->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
                break;
            default: 
                TextBlock_PlayerMatchResult->SetText(FText::FromString(TEXT("Survived")));
                TextBlock_PlayerMatchResult->SetColorAndOpacity(FSlateColor(FLinearColor::White));
                break;
            }
        }
    }

    // --- 2. 모든 플레이어의 통계 목록 표시 ---
    if (VerticalBox_KillerTeam)  VerticalBox_KillerTeam->ClearChildren();
    if (VerticalBox_SurvivorTeam)  VerticalBox_SurvivorTeam->ClearChildren();

    for (AInGamePlayerState* PS : AllPlayerStates)
    {
        if (PS && PlayerResultEntryWidgetClass)
        {
            UPlayerResultEntryWidget* EntryWidget = CreateWidget<UPlayerResultEntryWidget>(this, PlayerResultEntryWidgetClass);
            if (EntryWidget)
            {
                EntryWidget->SetPlayerData(PS);
                if (PS->bIsKiller) { if (VerticalBox_KillerTeam)  VerticalBox_KillerTeam->AddChildToVerticalBox(EntryWidget); }
                else { if (VerticalBox_SurvivorTeam)  VerticalBox_SurvivorTeam->AddChildToVerticalBox(EntryWidget); }
            }
        }
    }

}

void UResultScreenWidget::OnBackToMenuClicked()
{   
    // 임시지정
    UGameplayStatics::OpenLevel(GetWorld(), FName("/Game/Levels/Maps/MainMenuMap"));
}
