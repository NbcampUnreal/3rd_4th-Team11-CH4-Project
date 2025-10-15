#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerResultEntryWidget.generated.h"

class UTextBlock;
class UImage;
class AInGamePlayerState;

UCLASS()
class THEELEVENTHHOUR_API UPlayerResultEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 이 위젯에 플레이어 정보를 채워 넣는 함수
	void SetPlayerData(AInGamePlayerState* PlayerState);


protected:
    // UMG 디자이너와 연결될 변수들
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_PlayerName; // 플레이어 이름

    // 🔪 [킬러 전용]
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_NumCaptured; // 감옥에 보낸 횟수
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_NumKilled; //  플레이어를 죽인 횟수

    // 🛠️ [생존자 전용]
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_NumGeneratorsRepaired; // 발전기 수리 횟수
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_NumChestsBroken;    // 상자 부순 횟수
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_NumPrisonsUnlocked; // 감옥 문 연 횟수
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_NumCapturedTimes;   // 감옥에 갇힌 횟수
};
