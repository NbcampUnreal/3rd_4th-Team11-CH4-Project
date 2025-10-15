#include "MatchTimerWidget.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameMode/GameState/InGameGameState.h"

void UMatchTimerWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (TimerImage)
    {
        // 동적 머티리얼 인스턴스(MID) 생성
        TimerMID = TimerImage->GetDynamicMaterial();
    }

    if (AInGameGameState* GS = GetWorld()->GetGameState<AInGameGameState>())
    {
        // GameState의 RemainingTime 초기값이 최대 시간입니다.
        MaxTime = GS->RemainingTime;
    }

}

void UMatchTimerWidget::UpdateTimer(int32 Time)
{
    // MID가 유효한지 확인
    if (TimerMID)
    {
        // 남은 시간(Time)을 전체 시간(MaxTime)으로 나누어 0~1 사이의 비율(Percent)을 계산
        const float PercentValue = FMath::Clamp(static_cast<float>(Time) / MaxTime, 0.0f, 1.0f);

        // 머티리얼의 'Percent'라는 이름의 스칼라 파라미터 값을 방금 계산한 비율로 업데이트
        TimerMID->SetScalarParameterValue(FName("Percent"), PercentValue);
    }
}
