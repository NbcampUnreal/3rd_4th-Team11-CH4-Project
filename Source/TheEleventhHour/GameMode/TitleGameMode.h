#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TitleGameMode.generated.h"

/**
 * ATitleGameMode
 * - Dedicated 서버일 경우 Title 맵이 시작되면 자동으로 WaitingLevel 로 전환
 */
UCLASS()
class THEELEVENTHHOUR_API ATitleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATitleGameMode();

protected:
	virtual void BeginPlay() override;
};
