#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InGameHUDWidget.generated.h"

class UMatchTimerWidget;
class UMatchStatusWidget;
class UTeamStatusWidget;
class UItemWidget;
class USkillWidget;
class AInGameGameState;
class AInGamePlayerState;

UCLASS()
class THEELEVENTHHOUR_API UInGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    void UpdateHUD(int32 RemainingTime, int32 GeneratorsRepaired, bool bExitOpened);
	
protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UMatchTimerWidget> MatchTimer;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UMatchStatusWidget> MatchStatus;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTeamStatusWidget> TeamStatus;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UItemWidget> Item;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USkillWidget> Skill;

    // 서버로부터 복제된 게임 상태 정보를 담을 포인터
    UPROPERTY(Transient) 
    TObjectPtr<AInGameGameState> CurrentGameState;

    UPROPERTY(Transient)
    TObjectPtr<AInGamePlayerState> CurrentPlayerState;
};
