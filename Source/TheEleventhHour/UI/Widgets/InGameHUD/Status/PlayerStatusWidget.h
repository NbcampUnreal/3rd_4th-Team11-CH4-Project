#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "PlayerStatusWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class THEELEVENTHHOUR_API UPlayerStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    // TeamStatusWidget에서 호출 -> 플레이어의 상태와 이름을 받아 UI를 업데이트
    void UpdateState(ESurvivorState NewState, const FString& PlayerName);

protected:

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> Image_StatusIcon;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_PlayerName;

    // ESurvivorState : Enum과 아이콘 텍스처를 연결하는 맵(Map) 변수입니다.
    UPROPERTY(EditAnywhere, Category = "UI")
    TMap<ESurvivorState, TObjectPtr<UTexture2D>> StatusTextures;
	
};
