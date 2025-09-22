#include "ItemSpawnVolume.h"

#include "Item/Chest/Chest.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Engine/World.h"

AItemSpawnVolume::AItemSpawnVolume()
{
    bReplicates = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    Volume = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume"));
    Volume->SetupAttachment(Root);
    Volume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AItemSpawnVolume::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(SpawnTimer, this, &AItemSpawnVolume::TrySpawnOne, SpawnInterval, true);
    }
}

int32 AItemSpawnVolume::CountAliveChests() const
{
    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AChest::StaticClass(), Found);
    return Found.Num();
}

bool AItemSpawnVolume::GetRandomPointInVolume(FVector& OutLoc) const
{
    const FVector Origin = Volume->Bounds.Origin;
    const FVector Extent = Volume->Bounds.BoxExtent;

    const FVector RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);

    // 네비게이션 가능한 지점으로 스냅
    if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
    {
        FNavLocation NavLoc;
        if (NavSys->GetRandomPointInNavigableRadius(RandomPoint, NavSearchRadius, NavLoc))
        {
            OutLoc = NavLoc.Location;
            return true;
        }
    }

    OutLoc = RandomPoint; // 네비가 없어도 그냥 스폰
    return true;
}

void AItemSpawnVolume::TrySpawnOne()
{
    if (!HasAuthority() || !ChestClass) return;
    if (CountAliveChests() >= MaxAlive) return;

    FVector SpawnLoc;
    if (!GetRandomPointInVolume(SpawnLoc)) return;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    GetWorld()->SpawnActor<AChest>(ChestClass, SpawnLoc, FRotator::ZeroRotator, Params);
}
