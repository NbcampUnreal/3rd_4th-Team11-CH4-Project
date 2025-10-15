#include "UI/Widgets/InGameHUD/Skill/SkillWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"

void USkillWidget::UpdateSkill(bool bInCanUse, float InRemainingCooldown, float InTotalCooldown, float InSkillDuration)
{
    // 두 프로그레스 바가 모두 유효한지 확인
    if (!Image_SkillIcon || !ProgressBar_InProgress || !ProgressBar_Cooldown) return;

    if (bInCanUse)
    {
        // --- 1. 스킬 사용 가능 (Ready) ---
        // 쿨타임 바를 100%로 채워서 보여주고, 사용 중 바는 숨김
        Image_SkillIcon->SetBrushFromTexture(SkillReadyImage);

        ProgressBar_InProgress->SetVisibility(ESlateVisibility::Hidden);
        ProgressBar_Cooldown->SetVisibility(ESlateVisibility::Visible);
        ProgressBar_Cooldown->SetFillColorAndOpacity(FLinearColor::White);
        ProgressBar_Cooldown->SetPercent(1.0f);
    }
    else
    {
        float CooldownOnlyDuration = InTotalCooldown - InSkillDuration;

        if (InRemainingCooldown > CooldownOnlyDuration)
        {
            // --- 2. 스킬 '사용 중' (InProgress) ---
            // 쿨타임 바는 숨기고, '오른쪽->왼쪽'으로 닳는 InProgress 바 표시
            Image_SkillIcon->SetBrushFromTexture(SkillInProgressImage);

            ProgressBar_Cooldown->SetVisibility(ESlateVisibility::Hidden);
            ProgressBar_InProgress->SetVisibility(ESlateVisibility::Visible);
            ProgressBar_InProgress->SetFillColorAndOpacity(FLinearColor::Red);

            // 남은 지속 시간을 계산 (100% -> 0%)
            float TimeLeftInProgress = InRemainingCooldown - CooldownOnlyDuration;
            float Percent = InSkillDuration > 0 ? (TimeLeftInProgress / InSkillDuration) : 0.0f;
            ProgressBar_InProgress->SetPercent(Percent);
        }
        else
        {
            // --- 3. 순수 '쿨타임' (Cooldown) ---
            // InProgress 바는 숨기고, '왼쪽->오른쪽'으로 차오르는 쿨타임 바 표시
            Image_SkillIcon->SetBrushFromTexture(SkillReadyImage);

            ProgressBar_InProgress->SetVisibility(ESlateVisibility::Hidden);
            ProgressBar_Cooldown->SetVisibility(ESlateVisibility::Visible);
            ProgressBar_Cooldown->SetFillColorAndOpacity(FLinearColor::White);

            // 지난 쿨타임을 계산 (0% -> 100%)
            float TimePassedInCooldown = CooldownOnlyDuration - InRemainingCooldown;
            float Percent = CooldownOnlyDuration > 0 ? (TimePassedInCooldown / CooldownOnlyDuration) : 1.0f;
            ProgressBar_Cooldown->SetPercent(Percent);
        }
    }
}

void USkillWidget::UpdateSprint(float CurrentSprint, float MaxSprint)
{
    if (ProgressBar_Sprint && MaxSprint > 0)
    {
        ProgressBar_Sprint->SetPercent(CurrentSprint / MaxSprint);
    }
}