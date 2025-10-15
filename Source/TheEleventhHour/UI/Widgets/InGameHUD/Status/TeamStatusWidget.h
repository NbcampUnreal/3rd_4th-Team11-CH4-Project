#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "TeamStatusWidget.generated.h"

class AInGamePlayerState;
class UPlayerStatusWidget;

UCLASS()
class THEELEVENTHHOUR_API UTeamStatusWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    void UpdateTeammates(const TArray<AInGamePlayerState*>& PlayerStates);

protected:
    // 디자이너에 배치된 5개의 개별 플레이어 아이콘 위젯
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UPlayerStatusWidget> PlayerIcon_0;
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UPlayerStatusWidget> PlayerIcon_1;
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UPlayerStatusWidget> PlayerIcon_2;
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UPlayerStatusWidget> PlayerIcon_3;
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UPlayerStatusWidget> PlayerIcon_4;

    UPROPERTY()
    TArray<TObjectPtr<UPlayerStatusWidget>> PlayerIconWidgets;
    virtual void NativeConstruct() override;
};
