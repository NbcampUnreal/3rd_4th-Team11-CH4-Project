#include "InGameHUDWidget.h"
#include "GameMode/GameState/InGameGameState.h"     
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "Kismet/GameplayStatics.h"

#include "UI/Widgets/InGameHUD/Status/MatchTimerWidget.h"
#include "UI/Widgets/InGameHUD/Status/MatchStatusWidget.h"
#include "UI/Widgets/InGameHUD/Status/TeamStatusWidget.h"
#include "UI/Widgets/InGameHUD/Item/ItemWidget.h"
#include "UI/Widgets/InGameHUD/Skill/SkillWidget.h"

void UInGameHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

	CurrentGameState = GetWorld()->GetGameState<AInGameGameState>();
	CurrentPlayerState = GetOwningPlayerState<AInGamePlayerState>();
}

void UInGameHUDWidget::SetRoleSpecificVisibility(bool bIsKiller)
{
	if (bIsKiller)
	{
		// 살인마라면, Item 위젯은 숨기고 Skill 위젯은 보여줍니다.
		if (Item) Item->SetVisibility(ESlateVisibility::Collapsed);
		if (Skill) Skill->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		// 생존자라면, Skill 위젯은 숨기고 Item 위젯은 보여줍니다.
		if (Skill) Skill->SetVisibility(ESlateVisibility::Collapsed);
		if (Item) Item->SetVisibility(ESlateVisibility::Visible);
	}
}

void UInGameHUDWidget::UpdateHUD(int32 RemainingTime, int32 GeneratorsRepaired, bool bExitOpened)
{
	// PlayerController로부터 받은 데이터를 각 하위 위젯으로 전달.

	// --- 매치 타이머 업데이트 ---
	if (MatchTimer)
	{
		MatchTimer->UpdateTimer(RemainingTime, bExitOpened);
	}

	// --- 매치 상태(발전기, 출구) 업데이트 ---
	if (MatchStatus)
	{
		// GameState에 발전기 최대 개수 변수(예: MaxGenerators)가 있다고 가정 (보통 5)
		if (CurrentGameState && GeneratorsRepaired >= 5)
		{
			// 발전기 수리가 완료되면 탈출구 상태를 표시
			MatchStatus->ShowExitGateStatus(bExitOpened);
		}
		else
		{
			// 아직 수리 중이면 발전기 개수를 표시
			MatchStatus->UpdateGeneratorCount(GeneratorsRepaired);
		}
	}

	// --- 팀 상태 업데이트(플레이어 아이콘) ---
	if (TeamStatus)
	{
		// 1. 이 함수 안에서 현재 GameState를 다시 한번 직접 가져옵니다.
		if (AInGameGameState* GS = GetWorld()->GetGameState<AInGameGameState>())
		{
			// 2. GameState의 PlayerArray(모든 플레이어 목록)를 가져옵니다.
			TArray<APlayerState*> PlayerArray = GS->PlayerArray;

			// 3. PlayerState 배열을 InGamePlayerState 배열로 안전하게 변환합니다.
			TArray<AInGamePlayerState*> InGamePlayerStates;
			for (APlayerState* PS : PlayerArray)
			{
				if (AInGamePlayerState* InGamePS = Cast<AInGamePlayerState>(PS))
				{
					InGamePlayerStates.Add(InGamePS);
				}
			}

			// 4. TeamStatus 위젯으로 이 최종 목록을 전달합니다.
			TeamStatus->UpdateTeammates(InGamePlayerStates);
		}
	}

	// --- 아이템 및 스킬 위젯 업데이트 ---

	if (Item)
	{
		// Item->UpdateItem();
	}

	if (Skill) 
	{
		// SkillWidget->UpdateSkill();
	}

}

void UInGameHUDWidget::UpdateMurdererWidgets(bool bCanUseSkill, float RemainingCooldown, float TotalCooldown, float SkillDuration, float CurrentSprint, float MaxSprint)
{
	if (Skill)
	{
		Skill->UpdateSkill(bCanUseSkill, RemainingCooldown, TotalCooldown, SkillDuration);
		Skill->UpdateSprint(CurrentSprint, MaxSprint);
	}
}