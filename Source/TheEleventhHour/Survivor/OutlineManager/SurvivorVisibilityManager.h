#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SurvivorVisibilityManager.generated.h"

UCLASS()
class THEELEVENTHHOUR_API ASurvivorVisibilityManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ASurvivorVisibilityManager();
	bool bIsLocalSurvivor = false;	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	// 생존자 감지 및 관리
	void OnActorSpawned(AActor* Actor);
	UFUNCTION()
	void OnActorDestroyed(AActor* DestroyedActor);

	UPROPERTY(EditDefaultsOnly, Category="Outline")
	UMaterialInstance* DefaultOutlineMaterial;

	TArray<TWeakObjectPtr<class AEHSurvivorCharacter>> SurvivorCharacters;

	FDelegateHandle OnActorSpawnedHandle;

private:
};
