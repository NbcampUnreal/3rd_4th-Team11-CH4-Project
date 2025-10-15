#include "PlayerStatusWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UPlayerStatusWidget::UpdateState(ESurvivorState NewState, const FString& PlayerName)
{
    // 1. 플레이어 이름을 TextBlock에 설정
    if (TextBlock_PlayerName)
    {
        TextBlock_PlayerName->SetText(FText::FromString(PlayerName));
    }

    // 2. TMap에서 현재 상태(NewState)에 맞는 텍스처를 검색
    if (Image_StatusIcon)
    {
        const TObjectPtr<UTexture2D>* FoundTexturePtr = StatusTextures.Find(NewState);

        if (FoundTexturePtr && *FoundTexturePtr)
        {
            Image_StatusIcon->SetBrushFromTexture(*FoundTexturePtr);
        }
        else
        {
            Image_StatusIcon->SetBrush(FSlateBrush());
        }
    }
}