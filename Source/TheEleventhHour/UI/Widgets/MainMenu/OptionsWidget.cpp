#include "OptionsWidget.h"
#include "Components/Button.h"
#include "GameMode/GameInstance/EHGameInstance.h"
#include "OptionRow.h"
#include "Components/TextBlock.h"

void UOptionsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    GameInstanceRef = Cast<UEHGameInstance>(GetGameInstance());

    OptionRows.Empty();
    if (WBP_OptionRow_Gamma) OptionRows.Add(WBP_OptionRow_Gamma);
    if (WBP_OptionRow_Volume) OptionRows.Add(WBP_OptionRow_Volume);
    if (WBP_OptionRow_Sensitivity) OptionRows.Add(WBP_OptionRow_Sensitivity);

    if (GameInstanceRef && OptionRows.Num() == 3)
    {
        if (OptionRows[0] && OptionRows[0]->TextBlock_Name)
        {
            OptionRows[0]->TextBlock_Name->SetText(FText::FromString(TEXT("GAMMA")));
            OptionRows[0]->CurrentValue = GameInstanceRef->GetGamma() / 10;
        }

        if (OptionRows[1] && OptionRows[1]->TextBlock_Name)
        {
            OptionRows[1]->TextBlock_Name->SetText(FText::FromString(TEXT("VOLUME")));
            OptionRows[1]->CurrentValue = GameInstanceRef->GetMasterVolume() / 10;
        }

        if (OptionRows[2] && OptionRows[2]->TextBlock_Name)
        {
            OptionRows[2]->TextBlock_Name->SetText(FText::FromString(TEXT("SENSITIVITY")));
            OptionRows[2]->CurrentValue = GameInstanceRef->GetMouseSensitivity() / 10;
        }
    }

    for (auto Row : OptionRows)
    {
        if (Row) Row->UpdateVisuals();
    }

    CurrentRowIndex = 0;
    bIsEditingValue = false;
    UpdateSelectionVisuals();

    if (Button_Back)
    {
        Button_Back->OnClicked.AddDynamic(this, &UOptionsWidget::OnBackButtonClicked);
    }

    // 다음 프레임에 포커스 설정 (타이밍 이슈 해결)
    FSimpleDelegate FocusDelegate = FSimpleDelegate::CreateUObject(this, &UOptionsWidget::SetKeyboardFocus);
    FTimerHandle DummyHandle;
    GetWorld()->GetTimerManager().SetTimer(DummyHandle, [this]() {
        if (this)
        {
            this->SetKeyboardFocus();
        }
        }, 0.01f, false);
}

FReply UOptionsWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();

    if (bIsEditingValue)
    {
        if (Key == EKeys::Left) { if (OptionRows.IsValidIndex(CurrentRowIndex) && OptionRows[CurrentRowIndex]) { OptionRows[CurrentRowIndex]->ChangeValue(-1); ApplySettingChange(); } }
        else if (Key == EKeys::Right) { if (OptionRows.IsValidIndex(CurrentRowIndex) && OptionRows[CurrentRowIndex]) { OptionRows[CurrentRowIndex]->ChangeValue(1); ApplySettingChange(); } }
        else if (Key == EKeys::Escape || Key == EKeys::Enter) { bIsEditingValue = false; UpdateSelectionVisuals(); }
    }
    else
    {
        if (Key == EKeys::Up) { CurrentRowIndex = FMath::Clamp(CurrentRowIndex - 1, 0, OptionRows.Num() - 1); UpdateSelectionVisuals(); }
        else if (Key == EKeys::Down) { CurrentRowIndex = FMath::Clamp(CurrentRowIndex + 1, 0, OptionRows.Num() - 1); UpdateSelectionVisuals(); }
        else if (Key == EKeys::Enter || Key == EKeys::Right) { bIsEditingValue = true; UpdateSelectionVisuals(); }
        else if (Key == EKeys::Escape) { OnBackButtonClicked(); }
    }
    return FReply::Handled();
}

void UOptionsWidget::UpdateSelectionVisuals()
{
    for (int32 i = 0; i < OptionRows.Num(); ++i)
    {
        if (OptionRows.IsValidIndex(i) && OptionRows[i] && OptionRows[i]->TextBlock_Name)
        {
            FLinearColor TextColor = (i == CurrentRowIndex) ? FLinearColor::White : FLinearColor(0.5f, 0.5f, 0.5f);
            OptionRows[i]->TextBlock_Name->SetColorAndOpacity(FSlateColor(TextColor));
        }
    }
}

void UOptionsWidget::ApplySettingChange()
{
    if (!GameInstanceRef || !OptionRows.IsValidIndex(CurrentRowIndex) || !OptionRows[CurrentRowIndex]) return;
    const int32 NewValue = OptionRows[CurrentRowIndex]->CurrentValue * 10;

    switch (CurrentRowIndex)
    {
    case 0: GameInstanceRef->SetGamma(NewValue); break;
    case 1: GameInstanceRef->SetMasterVolume(NewValue); break;
    case 2: GameInstanceRef->SetMouseSensitivity(NewValue); break;
    }
}

void UOptionsWidget::OnBackButtonClicked()
{
    if (GameInstanceRef) { GameInstanceRef->HideOptionsMenu(); }
}