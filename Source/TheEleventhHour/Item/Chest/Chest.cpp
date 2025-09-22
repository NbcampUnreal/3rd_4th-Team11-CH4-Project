#include "Chest.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "Item/ItemPickup/ItemPickup.h"

AChest::AChest()
{
    bReplicates = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Root);
    Mesh->SetIsReplicated(true);

    HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
    HitBox->SetupAttachment(Root);
    HitBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    HitBox->SetCollisionObjectType(ECC_WorldDynamic);
    HitBox->SetCollisionResponseToAllChannels(ECR_Block);
    HitBox->OnComponentHit.AddDynamic(this, &AChest::OnHit);
}

void AChest::BeginPlay()
{
    Super::BeginPlay();
    CurrentHP = MaxHP;
}

void AChest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AChest, CurrentHP);
}

void AChest::OnHit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp,
                   FVector NormalImpulse, const FHitResult& Hit)
{
    if (HasAuthority())
        if (Other && Other->IsA(APawn::StaticClass()))
        {
            return;
        }
    if (!OtherComp || !OtherComp->IsSimulatingPhysics())
    {
        return;
    }

    FDamageEvent DamageEvent;
    TakeDamage(5.f, DamageEvent, nullptr, Other);
}

float AChest::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                         AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority() || CurrentHP <= 0.f) return 0.f;
    
    UE_LOG(LogTemp, Warning, TEXT("Chest::TakeDamage on SERVER, HP: %f -> %f, By: %s"),
       CurrentHP, FMath::Max(0.f, CurrentHP - DamageAmount),
       *GetNameSafe(DamageCauser));
    
    CurrentHP = FMath::Max(0.f, CurrentHP - DamageAmount);
    if (CurrentHP <= 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Chest BROKEN, calling ServerBreakChest"));
        ServerBreakChest();
    }
    return DamageAmount;
}

TSubclassOf<AItemPickup> AChest::PickRandomLoot() const
{
    const float Sum = KeyWeight + HealWeight + SpeedWeight;
    if (Sum <= KINDA_SMALL_NUMBER)
    {
        return nullptr;
    }

    const float R = FMath::FRandRange(0.f, Sum);
    float Acc = KeyWeight;

    if (R <= Acc) return KeyPickupClass;

    Acc += HealWeight;
    if (R <= Acc) return HealPickupClass;

    return SpeedPickupClass;
}

void AChest::ServerBreakChest_Implementation()
{
    // 드랍
    if (TSubclassOf<AItemPickup> DropClass = PickRandomLoot())
    {
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        const FVector Loc = GetActorLocation() + FVector(0,0,40);
        const FRotator Rot = FRotator::ZeroRotator;

        GetWorld()->SpawnActor<AItemPickup>(DropClass, Loc, Rot, Params);
    }

    // FX/SFX
    if (BreakFX)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BreakFX, GetActorTransform());
    }
    if (BreakSFX)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BreakSFX, GetActorLocation());
    }

    Destroy();
}
