#include "ResultScreenWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UResultScreenWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Button_BackToMenu)
    {
        Button_BackToMenu->OnClicked.AddDynamic(this, &UResultScreenWidget::OnBackToMenuClicked);
    }
}

void UResultScreenWidget::SetResultText(const FString& InResultText)
{
    // TextBlock_ResultTitle 위젯이 유효한지 확인하고 텍스트를 설정.
    if (TextBlock_ResultTitle)
    {
        TextBlock_ResultTitle->SetText(FText::FromString(InResultText));
    }
}

void UResultScreenWidget::OnBackToMenuClicked()
{
    // 메인 메뉴 레벨 Open
    UGameplayStatics::OpenLevel(GetWorld(), FName("MainMenuMap"));
}

