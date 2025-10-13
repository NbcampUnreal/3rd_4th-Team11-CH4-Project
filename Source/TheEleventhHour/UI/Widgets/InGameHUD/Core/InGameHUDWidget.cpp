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

    // 위젯이 처음 생성될 때, GameState와 PlayerState에 대한 참조
    // 이 객체들은 서버로부터 데이터를 복제(Replication)
    CurrentGameState = GetWorld()->GetGameState<AInGameGameState>();
    CurrentPlayerState = GetOwningPlayerState<AInGamePlayerState>();
}

void UInGameHUDWidget::UpdateHUD(int32 RemainingTime, int32 GeneratorsRepaired, bool bExitOpened)
{
	// PlayerController로부터 받은 데이터를 각 하위 위젯으로 전달.

	// --- 매치 타이머 업데이트 ---
	if (MatchTimer)
	{
		MatchTimer->UpdateTimer(RemainingTime);
	}

	// --- 매치 상태(발전기, 출구) 업데이트 ---
	if (MatchStatus)
	{
	//	MatchStatus->UpdateGeneratorCount(GeneratorsRepaired);
	//	MatchStatus->UpdateExitGateStatus(bExitOpened);
	}

	// --- 팀 상태 업데이트 ---
	if (TeamStatus)
	{
	//	TeamStatus->UpdateTeammates(TeammateInfos);
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