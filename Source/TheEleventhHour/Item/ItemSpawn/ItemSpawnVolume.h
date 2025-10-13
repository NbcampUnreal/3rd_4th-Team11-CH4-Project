#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemSpawnVolume.generated.h"

class UBoxComponent;
class AChest;
class AItemPickup;

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

	UPROPERTY(EditAnywhere, Category="Spawn|Items")
	TArray<TSubclassOf<AItemPickup>> ItemClasses;

	UPROPERTY(EditAnywhere, Category="Spawn|Items")
	float SpawnCooldown = 60.f; // 1분 기본

	bool bCanSpawn = true;
	FTimerHandle SpawnCooldownHandle;

	UFUNCTION()
	void SpawnItem();

	UFUNCTION()
	void ResetSpawn();

	FVector GetSpawnLocation() const;

	UPROPERTY(EditAnywhere, Category="Spawn|Chest")
	float ChestRespawnCooldown = 60.f;   // 상자 파괴 후 다음 스폰까지 최소 대기

	bool bChestRespawnCooling = false;
	FTimerHandle ChestRespawnCooldownHandle;

	UPROPERTY(VisibleInstanceOnly, Category="Spawn")
	TWeakObjectPtr<class AChest> AliveChest;


	// 체스트 파괴 시 콜백 (AActor::OnDestroyed)
	UFUNCTION()
	void OnChestDestroyed(AActor* DestroyedActor);

	// 쿨다운 종료
	UFUNCTION()
	void EndChestRespawnCooldown();
};
