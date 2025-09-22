#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemSpawnVolume.generated.h"

class UBoxComponent;
class AChest;

UCLASS()
class THEELEVENTHHOUR_API AItemSpawnVolume : public AActor
{
	GENERATED_BODY()

public:
	AItemSpawnVolume();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category="Spawn|Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawn|Components")
	UBoxComponent* Volume;

	UPROPERTY(EditAnywhere, Category="Spawn")
	TSubclassOf<AChest> ChestClass;

	UPROPERTY(EditAnywhere, Category="Spawn")
	int32 MaxAlive = 10;

	UPROPERTY(EditAnywhere, Category="Spawn")
	float SpawnInterval = 8.f;

	UPROPERTY(EditAnywhere, Category="Spawn")
	float NavSearchRadius = 2000.f;

	FTimerHandle SpawnTimer;

	UFUNCTION()
	void TrySpawnOne();

	int32 CountAliveChests() const;
	bool GetRandomPointInVolume(FVector& OutLoc) const;
};
