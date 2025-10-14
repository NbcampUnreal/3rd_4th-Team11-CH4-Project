#include "Chest.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Item/ItemPickup/ItemPickup.h"
#include "Item/ItemLog/ItemLog.h"
#include "Murderer/MurdererCharacter.h"

AChest::AChest()
{
    bReplicates = true;
    bAlwaysRelevant = true;

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
    if (HasAuthority())
    {
        // 1) 초기 충돌 OFF (스폰 즉시 튕김 방지)
        SetActorEnableCollision(false);
        if (HitBox)
        {
            // HitBox는 기본 BlockAll에 가까운 설정입니다. :contentReference[oaicite:4]{index=4}
            HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        // 2) 지면으로 스냅
        SnapToGround();

        // 3) 다음 틱에 충돌 ON
        GetWorldTimerManager().SetTimerForNextTick(this, &AChest::EnableCollisionDeferred);
    }
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
    if (!HasAuthority() || CurrentHP <= 0.f)
    {
        return 0.f;
    }
    // 살인마일 때 무시
    if (bIgnoreDamageFromMurderer && DamageCauser && DamageCauser->IsA(AMurdererCharacter::StaticClass()))
    {
        return 0.f;
    }
    
    const float OldHP = CurrentHP;
    
    const float ValidDamage = FMath::IsFinite(DamageAmount) 
        ? FMath::Max(0.f, DamageAmount) 
        : 0.f;

    if (ValidDamage <= 0.f)
    {
        return 0.f;
    }
    
    const float NewHP = FMath::Clamp(OldHP - ValidDamage, 0.f, MaxHP);
    
    if (NewHP < OldHP)
    {
        CurrentHP = NewHP;
        MulticastPlayHitSFX();       
        MulticastStartDamageLoop(); 

        GetWorldTimerManager().ClearTimer(DamageLoopStopTimer);
        GetWorldTimerManager().SetTimer(
            DamageLoopStopTimer, this, &AChest::MulticastStopDamageLoop,
            DamageLoopStopDelay, false
        );
    }
    else
    {
        return 0.f;
    }
    if (CurrentHP <= 0.f)
    {
        ServerBreakChest();
    }
    return ValidDamage;
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
    
    SetNetDormancy(DORM_Awake);
    ForceNetUpdate();
    
    MulticastPlayBreakSFX();
    
    if (BreakFX)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BreakFX, GetActorTransform());
    }
    Destroy();
}
// ========== SFX 멀티캐스트들 ==========

void AChest::MulticastPlayHitSFX_Implementation()
{
    if (HitSFX)
    {
        // 모든 클라에서 로컬 재생
        UGameplayStatics::PlaySoundAtLocation(this, HitSFX, GetActorLocation());
    }
}

void AChest::MulticastStartDamageLoop_Implementation()
{
    if (!DamageLoopSFX) return;

    // 이미 재생 중이면 유지
    if (DamageLoopComp && DamageLoopComp->IsPlaying()) return;

    // 없으면 새로 붙여 재생(루프 On)
    if (!DamageLoopComp)
    {
        DamageLoopComp = UGameplayStatics::SpawnSoundAttached(
            DamageLoopSFX, RootComponent, NAME_None,
            FVector::ZeroVector, EAttachLocation::SnapToTarget,
            /*bStopWhenAttachedToDestroyed*/ true
        );
    }

    if (DamageLoopComp && !DamageLoopComp->IsPlaying())
    {
        DamageLoopComp->Play();
    }
}

void AChest::MulticastStopDamageLoop_Implementation()
{
    if (DamageLoopComp)
    {
        // 살짝 페이드아웃 후 정지
        DamageLoopComp->FadeOut(0.1f, 0.f);
    }
}

void AChest::MulticastPlayBreakSFX_Implementation()
{
    if (BreakSFX)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BreakSFX, GetActorLocation());
    }
}
void AChest::SnapToGround()
{
    const FVector Start = GetActorLocation();
    const FVector End   = Start - FVector(0, 0, GroundTraceDown);

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(ChestGroundTrace), false, this);

    // 지면 히트 시, 살짝 띄워서 위치 고정
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        const FVector NewLoc = Hit.ImpactPoint + FVector(0, 0, GroundSnapOffset);
        SetActorLocation(NewLoc, false, nullptr, ETeleportType::TeleportPhysics);
    }
}

void AChest::EnableCollisionDeferred()
{
    SetActorEnableCollision(true);
    if (HitBox)
    {
        HitBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}