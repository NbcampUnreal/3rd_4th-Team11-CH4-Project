#include "ItemSpawnVolume.h"
#include "Item/Chest/Chest.h"
#include "Item/ItemPickup/ItemPickup.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Item/ItemLog/ItemLog.h"

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
            OutLoc.Z = GetActorLocation().Z;
            return true;
        }
    }

    OutLoc = RandomPoint; // 네비가 없어도 그냥 스폰
    OutLoc.Z = GetActorLocation().Z;
    return true;
}

void AItemSpawnVolume::TrySpawnOne()
{
    if (!HasAuthority() || !ChestClass) return;
    if (bChestRespawnCooling) return;

    const int32 Alive = CountAliveChests();
    if (Alive >= MaxAlive) return;

    FVector SpawnLoc;
    if (!GetRandomPointInVolume(SpawnLoc)) return;

    // 지면 투영
    {
        FHitResult Hit;
        const FVector Start = SpawnLoc + FVector(0, 0, 200.f);
        const FVector End   = SpawnLoc - FVector(0, 0, 2000.f);
        FCollisionQueryParams Params(SCENE_QUERY_STAT(SpawnVolumeTrace), false, this);

        if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
        {
            SpawnLoc = Hit.ImpactPoint + FVector(0, 0, 6.f); // 살짝 띄워서 바닥 관통 방지
        }
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AChest* Spawned = GetWorld()->SpawnActor<AChest>(ChestClass, SpawnLoc, FRotator::ZeroRotator, Params);
    if (Spawned)
    {
        Spawned->OnDestroyed.AddDynamic(this, &AItemSpawnVolume::OnChestDestroyed);
    }
}

void AItemSpawnVolume::SpawnItem()
{
    if (!bCanSpawn) return;

    if (ItemClasses.Num() > 0)
    {
        int32 Index = FMath::RandRange(0, ItemClasses.Num() - 1);
        if (TSubclassOf<AItemPickup> ItemClass = ItemClasses[Index])
        {
            FVector SpawnLocation = GetSpawnLocation();
            GetWorld()->SpawnActor<AItemPickup>(ItemClass, SpawnLocation, FRotator::ZeroRotator);
        }
    }

    bCanSpawn = false;
    GetWorld()->GetTimerManager().SetTimer(
        SpawnCooldownHandle,
        this,
        &AItemSpawnVolume::ResetSpawn,
        SpawnCooldown,
        false
    );
}

void AItemSpawnVolume::ResetSpawn()
{
    bCanSpawn = true;
}

FVector AItemSpawnVolume::GetSpawnLocation() const
{
    if (!Volume)
    {
        return GetActorLocation();
    }

    const FBoxSphereBounds Bounds = Volume->Bounds;
    return UKismetMathLibrary::RandomPointInBoundingBox(Bounds.Origin, Bounds.BoxExtent);
}

void AItemSpawnVolume::OnChestDestroyed(AActor* DestroyedActor)
{
    // 이미 쿨다운 중이면 무시
    if (bChestRespawnCooling) return;

    bChestRespawnCooling = true;
    GetWorld()->GetTimerManager().SetTimer(
        ChestRespawnCooldownHandle,
        this,
        &AItemSpawnVolume::EndChestRespawnCooldown,
        ChestRespawnCooldown,
        false
    );
}

// 쿨다운 종료
void AItemSpawnVolume::EndChestRespawnCooldown()
{
    bChestRespawnCooling = false;
}