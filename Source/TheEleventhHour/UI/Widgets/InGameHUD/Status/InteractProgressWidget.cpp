#include "UI/Widgets/InGameHUD/Status/InteractProgressWidget.h"
#include "Components/ProgressBar.h"

void UInteractProgressWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ProgressBar_Repair)
	{
		ProgressBar_Repair->SetPercent(0.f);
	}
}

void UInteractProgressWidget::SetProgress(float InProgress)
{
	if (ProgressBar_Repair)
	{
		ProgressBar_Repair->SetPercent(FMath::Clamp(InProgress, 0.f, 1.f));
	}
}
