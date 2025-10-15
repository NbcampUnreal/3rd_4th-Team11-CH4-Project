#include "OptionRow.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Border.h"

void UOptionRow::NativeConstruct()
{
    Super::NativeConstruct();

    if (HorizontalBox_Values)
    {
        ValueBoxes.Empty();
        for (UWidget* Child : HorizontalBox_Values->GetAllChildren())
        {
            if (UBorder* Border = Cast<UBorder>(Child))
            {
                ValueBoxes.Add(Border);
            }
        }
    }
    UpdateVisuals();
}

void UOptionRow::UpdateVisuals()
{
    for (int32 i = 0; i < ValueBoxes.Num(); ++i)
    {
        if (ValueBoxes.IsValidIndex(i) && ValueBoxes[i])
        {
            FLinearColor Color = (i < CurrentValue) ? FLinearColor::White : FLinearColor(0.1f, 0.1f, 0.1f);
            ValueBoxes[i]->SetBrushColor(Color);
        }
    }
}

void UOptionRow::ChangeValue(int32 Amount)
{
    CurrentValue = FMath::Clamp(CurrentValue + Amount, 0, 10);
    UpdateVisuals();
}