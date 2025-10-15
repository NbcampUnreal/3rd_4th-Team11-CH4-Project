#include "Item/ItemPickup/ItemPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Item/Inventory/InventoryComponent.h"
#include "Item/ItemLog/ItemLog.h"

AItemPickup::AItemPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bCanBePickedUp = false;
	
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SetRootComponent(Sphere);
	Sphere->InitSphereRadius(60.f);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AItemPickup::OnOverlap);
}
void AItemPickup::BeginPlay()
{
	Super::BeginPlay();

	// 1.5초 뒤에 EnablePickup 실행
	GetWorldTimerManager().SetTimer(PickupEnableTimer, this, &AItemPickup::EnablePickup, PickupDelaySeconds, false);

	if (HasAuthority() && AutoDespawnSeconds > 0.f)
   {
		GetWorldTimerManager().SetTimer(DespawnTimer, this, &AItemPickup::DespawnSelf, AutoDespawnSeconds, false);
   }
}

void AItemPickup::EnablePickup()
{
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	bCanBePickedUp = true;

	if (HasAuthority())
	{
		TArray<AActor*> Overlapping;
		Sphere->GetOverlappingActors(Overlapping);
		for (AActor* O : Overlapping)
		{
			if (TryPickup(O))
			{
				break;
			}
		}
	}
}

bool AItemPickup::TryPickup(AActor* Other)
{
	if (!HasAuthority() || !bCanBePickedUp || !IsValid(Other)) return false;

	if (UInventoryComponent* Inv = Other->FindComponentByClass<UInventoryComponent>())
	{
		if (Inv->AddItem(ItemType))
		{
			// UI용 이벤트 브로드 캐스트
			OnItemPicked.Broadcast(ItemType);
			
			GetWorldTimerManager().ClearTimer(DespawnTimer);
			Destroy();
			return true;
		}
	}
	return false;
}

void AItemPickup::OnOverlap(UPrimitiveComponent* Overlapped, AActor* Other,
	UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Hit)
{
	if (!HasAuthority() || !bCanBePickedUp) return;

	TryPickup(Other);
}
void AItemPickup::DespawnSelf()
{
	if (!HasAuthority()) return;
	Destroy();
}