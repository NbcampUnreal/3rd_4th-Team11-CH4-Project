#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UIManagerComponent.generated.h"

class UAudioCompoent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THEELEVENTHHOUR_API UUIManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	void BindDelegates();
	void PlayUISound();

private:
	class UAudioComponent* UIAudioComponent;
};
