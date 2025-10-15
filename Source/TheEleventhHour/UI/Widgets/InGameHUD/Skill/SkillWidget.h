#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "SkillWidget.generated.h"

class UImage;
class UProgressBar;

UCLASS()
class THEELEVENTHHOUR_API USkillWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    void UpdateSkill(bool bInCanUse, float InRemainingCooldown, float InTotalCooldown, float InSkillDuration);
    void UpdateSprint(float CurrentSprint, float MaxSprint);

protected:
    // 스킬 관련 위젯들
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage> Image_SkillIcon;

    UPROPERTY(meta = (BindWidget)) TObjectPtr<UProgressBar> ProgressBar_InProgress;
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UProgressBar> ProgressBar_Cooldown;
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UProgressBar> ProgressBar_Sprint;

    UPROPERTY(EditAnywhere, Category = "Skill UI") TObjectPtr<UTexture2D> SkillReadyImage;
    UPROPERTY(EditAnywhere, Category = "Skill UI") TObjectPtr<UTexture2D> SkillInProgressImage;
	
};
