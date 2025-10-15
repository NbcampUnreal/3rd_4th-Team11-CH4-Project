#include "Chest.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "GameMode/InGameGameMode.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "Item/ItemPickup/ItemPickup.h"
#include "Item/ItemLog/ItemLog.h"
#include "Murderer/MurdererCharacter.h"
#include "Survivor/Character/EHSurvivorCharacter.h"

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
        return 0.f;

    // 🔹 살인마가 상자 부수는 건 무시
    if (bIgnoreDamageFromMurderer && DamageCauser && DamageCauser->IsA(AMurdererCharacter::StaticClass()))
    {
        return 0.f;
    }

    const float OldHP = CurrentHP;
    const float ValidDamage = FMath::Max(0.f, DamageAmount);
    CurrentHP = FMath::Clamp(CurrentHP - ValidDamage, 0.f, MaxHP);

    if (CurrentHP < OldHP)
    {
        // 🔹 히트 사운드 / 루프 사운드
        MulticastPlayHitSFX();
        MulticastStartDamageLoop();

        GetWorldTimerManager().ClearTimer(DamageLoopStopTimer);
        GetWorldTimerManager().SetTimer(
            DamageLoopStopTimer, this, &AChest::MulticastStopDamageLoop,
            DamageLoopStopDelay, false
        );
    }

    // 🔹 파괴 판정
    if (CurrentHP <= 0.f)
    {
        AEHSurvivorCharacter* Survivor = Cast<AEHSurvivorCharacter>(DamageCauser);
        if (Survivor)
        {
            if (AInGamePlayerState* PS = Survivor->GetPlayerState<AInGamePlayerState>())
            {
                PS->AddChestBroken();
                //UE_LOG(LogTemp, Log, TEXT("[Chest] %s broke a chest (total: %d)"),
                //    *PS->GetPlayerName(), PS->NumChestsBroken);

                if (AInGameGameMode* GM = GetWorld()->GetAuthGameMode<AInGameGameMode>())
                {
                    GM->LogAllPlayerStates();
                }
            }
        }

        ServerBreakChest(); // 🔹 원래 기능 유지
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
        //UE_LOG(LogItem, Log, TEXT("[Chest] 파괴음 멀티캐스트 호출: %s, 사운드=%s"),HasAuthority() ? TEXT("서버") : TEXT("클라이언트"),*BreakSFX->GetName());
        UGameplayStatics::PlaySoundAtLocation(this, BreakSFX, GetActorLocation());
    }
    else
    {
        //UE_LOG(LogItem, Warning, TEXT("[Chest] 파괴음 멀티캐스트 호출: %s, 하지만 BreakSFX가 비어있음"),
        //   HasAuthority() ? TEXT("서버") : TEXT("클라이언트"));
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
        //UE_LOG(LogItem, Log, TEXT("[Chest] 지면 스냅 적용: %s → Z=%.1f"), *GetName(), NewLoc.Z);
    }
    else
    {
        //UE_LOG(LogItem, Warning, TEXT("[Chest] 지면 스냅 실패: %s (트레이스 미히트)"), *GetName());
    }
}

void AChest::EnableCollisionDeferred()
{
    SetActorEnableCollision(true);
    if (HitBox)
    {
        HitBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    //UE_LOG(LogItem, Log, TEXT("[Chest] 충돌 재활성화"));
}