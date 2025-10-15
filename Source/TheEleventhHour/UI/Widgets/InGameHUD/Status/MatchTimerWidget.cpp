#include "MatchTimerWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameMode/GameState/InGameGameState.h"

void UMatchTimerWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (TextBlock_Countdown) { TextBlock_Countdown->SetVisibility(ESlateVisibility::Hidden); }

    // 위젯이 생성될 때, 현재 월드의 GameState를 가져옵니다.
    if (AInGameGameState* GS = GetWorld()->GetGameState<AInGameGameState>())
    {
        // GameState에 설정된 초기 남은 시간 값을 MaxTime 변수에 저장합니다.
        MaxTime = GS->RemainingTime;
    }
}


void UMatchTimerWidget::UpdateTimer(int32 RemainingTime, bool bExitOpened)
{
    if (!ProgressBar_Timer || !TextBlock_Countdown) return;

    // 1. 퍼센트 계산 및 프로그레스 바 업데이트
    const float Percent = (MaxTime > 0) ? FMath::Clamp(static_cast<float>(RemainingTime) / MaxTime, 0.0f, 1.0f) : 0.0f;
    ProgressBar_Timer->SetPercent(Percent);

    // 2. 색상 및 카운트다운 텍스트 처리
    if (RemainingTime <= 60 || bExitOpened)
    {
        // 60초 이하이거나 문이 열리면: 빨간색 + 카운트다운 표시
        ProgressBar_Timer->SetFillColorAndOpacity(FLinearColor::Red);
        TextBlock_Countdown->SetVisibility(ESlateVisibility::HitTestInvisible);
        TextBlock_Countdown->SetText(FText::AsNumber(RemainingTime));

        // 펄스 애니메이션이 재생 중이 아니라면, 반복 재생 시작
        if (PulseAnim && !IsAnimationPlaying(PulseAnim))
        {
            PlayAnimation(PulseAnim, 0.0f, 0); // 0 = 무한 반복
        }
    }
    else
    {
        // 그 외의 경우: 카운트다운 숨기고, 퍼센트에 따라 색상 변경
        TextBlock_Countdown->SetVisibility(ESlateVisibility::Hidden);
        if (IsAnimationPlaying(PulseAnim))
        {
            StopAnimation(PulseAnim);
        }

        if (Percent > 0.66f)
        {
            ProgressBar_Timer->SetFillColorAndOpacity(FLinearColor::White);
        }
        else if (Percent > 0.33f)
        {
            ProgressBar_Timer->SetFillColorAndOpacity(FLinearColor::Yellow);
        }
        else
        {
            ProgressBar_Timer->SetFillColorAndOpacity(FLinearColor(1.0f, 0.5f, 0.0f)); // 주황색
        }
    }
}