#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/ItemTypes/ItemTypes.h"
#include "ItemPickup.generated.h"

UCLASS()
class THEELEVENTHHOUR_API AItemPickup : public AActor
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemPicked, EItemType, Item);

	UPROPERTY(BlueprintAssignable, Category="Pickup")
	FOnItemPicked OnItemPicked;
	
	AItemPickup();

protected:
	
	UPROPERTY(VisibleAnywhere) class USphereComponent* Sphere;
	UPROPERTY(VisibleAnywhere) class UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category="Item")
	EItemType ItemType = EItemType::Key;

	UPROPERTY(EditAnywhere, Category="Pickup")
	float PickupDelaySeconds = 1.0f;

	UPROPERTY(VisibleInstanceOnly, Category="Pickup")
	bool bCanBePickedUp;

	FTimerHandle PickupEnableTimer;

	UPROPERTY(EditAnywhere, Category="Pickup")
	float AutoDespawnSeconds = 10.f;   // 원하는 기본값
	FTimerHandle DespawnTimer;

	UFUNCTION()
	void DespawnSelf();

	virtual void BeginPlay() override;

	UFUNCTION()
	void EnablePickup();

	bool TryPickup(AActor* Other);
	
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* Overlapped, AActor* Other,
				   UPrimitiveComponent* OtherComp, int32 BodyIndex,
				   bool bFromSweep, const FHitResult& Hit);
};
