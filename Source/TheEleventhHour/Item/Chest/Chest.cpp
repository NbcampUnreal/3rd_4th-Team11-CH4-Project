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
        UE_LOG(LogTemp, Warning,
            TEXT("Chest::TakeDamage SKIPPED (HasAuthority=%d, CurrentHP=%.1f)"),
            HasAuthority(), CurrentHP);
        return 0.f;
    }
    // 살인마일 때 무시
    if (bIgnoreDamageFromMurderer && DamageCauser && DamageCauser->IsA(AMurdererCharacter::StaticClass()))
    {
        UE_LOG(LogTemp, Warning, TEXT("Chest::TakeDamage IGNORED (Murderer cannot break chests)"));
        return 0.f;
    }
    
    const float OldHP = CurrentHP;
    
    const float ValidDamage = FMath::IsFinite(DamageAmount) 
        ? FMath::Max(0.f, DamageAmount) 
        : 0.f;

    if (ValidDamage <= 0.f)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Chest::TakeDamage IGNORED (Invalid Damage=%.1f)"),
            DamageAmount);
        return 0.f;
    }
    
    const float NewHP = FMath::Clamp(OldHP - ValidDamage, 0.f, MaxHP);
    
    if (NewHP < OldHP)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Chest::TakeDamage SUCCESS -> HP: %.1f -> %.1f | Damage: %.1f | Causer: %s"),
            OldHP, NewHP, ValidDamage, *GetNameSafe(DamageCauser));

        CurrentHP = NewHP;
        MulticastPlayHitSFX();       // 한 번 ‘딱’ 소리
        MulticastStartDamageLoop();  // 연타 동안 ‘웅—’ 유지

        GetWorldTimerManager().ClearTimer(DamageLoopStopTimer);
        GetWorldTimerManager().SetTimer(
            DamageLoopStopTimer, this, &AChest::MulticastStopDamageLoop,
            DamageLoopStopDelay, false
        );
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Chest::TakeDamage NO CHANGE (Damage=%.1f did not reduce HP)"),
            ValidDamage);
        return 0.f;
    }
    if (CurrentHP <= 0.f)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Chest::TakeDamage CHEST BROKEN! Calling ServerBreakChest()"));
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

    // FX/SFX
    SetNetDormancy(DORM_Awake);
    ForceNetUpdate();

    // 3) 🔊 파괴음: 모든 클라에서 Reliable 멀티캐스트로 위치 재생
    MulticastPlayBreakSFX();

    // 4) (선택) FX — 모든 클라에서 보이고 싶으면 멀티캐스트 함수로 빼는 걸 권장
    if (BreakFX)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BreakFX, GetActorTransform());
    }

    // 5) 마지막에 파괴
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
        UE_LOG(LogItem, Log, TEXT("[Chest] 파괴음 멀티캐스트 호출: %s, 사운드=%s"),
    HasAuthority() ? TEXT("서버") : TEXT("클라이언트"),
    *BreakSFX->GetName());
        
        UGameplayStatics::PlaySoundAtLocation(this, BreakSFX, GetActorLocation());
    }
    else
    {
        UE_LOG(LogItem, Warning, TEXT("[Chest] 파괴음 멀티캐스트 호출: %s, 하지만 BreakSFX가 비어있음"),
            HasAuthority() ? TEXT("서버") : TEXT("클라이언트"));
    }
}