#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BackgroundMusicActor.generated.h"

class UAudioComponent;
class USoundBase;

UCLASS()
class THEELEVENTHHOUR_API ABackgroundMusicActor : public AActor
{
	GENERATED_BODY()
	
public:    
	ABackgroundMusicActor();

	UFUNCTION()
	void PlayBGMSound();
	UFUNCTION()
	void StopBGMSound();
	
protected:
	virtual void BeginPlay() override;

public:    
	// 오디오 컴포넌트 (배경음악 재생용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Audio")
	UAudioComponent* AudioComponent;

	// 재생할 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio")
	USoundBase* BackgroundMusic;
};
