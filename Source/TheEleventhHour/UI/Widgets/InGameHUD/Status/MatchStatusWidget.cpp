#include "MatchStatusWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UMatchStatusWidget::UpdateGeneratorCount(int32 Count)
{
    Image_GeneratorIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
    TextBlock_GeneratorCount->SetVisibility(ESlateVisibility::HitTestInvisible);
    Image_ExitGateIcon->SetVisibility(ESlateVisibility::Collapsed);

    // 텍스트를 "x / 5" 형태로 업데이트합니다.
    FText GenText = FText::FromString(FString::Printf(TEXT("%d"), 5 - Count));
    TextBlock_GeneratorCount->SetText(GenText);
}

void UMatchStatusWidget::ShowExitGateStatus(bool bIsGateOpened)
{
    // 발전기 아이콘과 텍스트는 숨기고, 탈출구 아이콘은 보이게 합니다.
    Image_GeneratorIcon->SetVisibility(ESlateVisibility::Collapsed);
    TextBlock_GeneratorCount->SetVisibility(ESlateVisibility::Collapsed);
    Image_ExitGateIcon->SetVisibility(ESlateVisibility::HitTestInvisible);

    // 탈출구 개방 상태에 따라 다른 아이콘 이미지를 보여줍니다.
    if (bIsGateOpened && ExitGateOpenedTexture)
    {
        Image_ExitGateIcon->SetBrushFromTexture(ExitGateOpenedTexture);
    }
    else if (ExitGateClosedTexture)
    {
        Image_ExitGateIcon->SetBrushFromTexture(ExitGateClosedTexture);
    }
}