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
	AItemPickup();

protected:
	UPROPERTY(VisibleAnywhere) class USphereComponent* Sphere;
	UPROPERTY(VisibleAnywhere) class UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category="Item")
	EItemType ItemType = EItemType::Key;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* Overlapped, AActor* Other,
				   UPrimitiveComponent* OtherComp, int32 BodyIndex,
				   bool bFromSweep, const FHitResult& Hit);
};
