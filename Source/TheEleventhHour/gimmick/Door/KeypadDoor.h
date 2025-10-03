#pragma once

#include "CoreMinimal.h"
#include "gimmick/Door/MultiplayDoor.h"
#include "KeypadDoor.generated.h"

UCLASS()
class THEELEVENTHHOUR_API AKeypadDoor : public AMultiplayDoor
{
	GENERATED_BODY()

public:
	AKeypadDoor();

	UFUNCTION()
	void OpenKeypadDoor();
};
