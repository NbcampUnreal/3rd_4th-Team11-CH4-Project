#include "Item/ItemPickup/ItemPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Item/Inventory/InventoryComponent.h"
#include "Item/ItemLog/ItemLog.h"

AItemPickup::AItemPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SetRootComponent(Sphere);
	Sphere->InitSphereRadius(60.f);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AItemPickup::OnOverlap);
}

void AItemPickup::OnOverlap(UPrimitiveComponent* Overlapped, AActor* Other,
	UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Hit)
{
	if (!Other) return;

	if (UInventoryComponent* Inv = Other->FindComponentByClass<UInventoryComponent>())
	{
		if (Inv->AddItem(ItemType))
		{
			UE_LOG(LogItem, Log, TEXT("[Pickup] %s 획득 → 아이템 제거"),
				*UEnum::GetValueAsString(ItemType));
			Destroy(); // 로컬: 즉시 제거
		}
		else
		{
			UE_LOG(LogItem, Warning, TEXT("[Pickup] 인벤토리 가득 → 미획득"));
		}
	}
}
