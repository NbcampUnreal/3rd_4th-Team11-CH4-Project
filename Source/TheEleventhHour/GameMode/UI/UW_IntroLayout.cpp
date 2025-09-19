#include "GameMode/UI/UW_IntroLayout.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Animation/WidgetAnimation.h"

void UUW_IntroLayout::NativeConstruct()
{
	Super::NativeConstruct();

	if (FadeIn)
	{
		PlayAnimation(FadeIn);
	}
	
	GetWorld()->GetTimerManager().SetTimer(
		FadeOutTimerHandle, this, &UUW_IntroLayout::StartFadeOut, 3.0f, false);
}

void UUW_IntroLayout::StartFadeOut()
{
	if (FadeOut)
	{
		float Duration = FadeOut->GetEndTime();

		PlayAnimation(FadeOut);
		
		FadeOutFinishedDelegate.Clear();
		FadeOutFinishedDelegate.BindDynamic(this, &UUW_IntroLayout::OnFadeOutFinished);

		if (Duration > 0.0f)
		{
			BindToAnimationFinished(FadeOut, FadeOutFinishedDelegate);
		}
		else
		{
			OnFadeOutFinished();
		}
	}
	else
	{
		OnFadeOutFinished();
	}
}

void UUW_IntroLayout::OnFadeOutFinished()
{
	UGameplayStatics::OpenLevel(this, "Title");
}