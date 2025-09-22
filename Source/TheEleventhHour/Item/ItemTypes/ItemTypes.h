// ItemTypes.h

#pragma once

#include "CoreMinimal.h"
#include "ItemTypes.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None UMETA(DisplayName = "None"),
	Key UMETA(DisplayName = "Key"),
	Heal UMETA(DisplayName = "Heal"),
	Speed UMETA(DisplayName = "Speed"),
};
