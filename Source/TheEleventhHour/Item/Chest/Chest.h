#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chest.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class AItemPickup;
class UParticleSystem;
class USoundBase;

UCLASS()
class THEELEVENTHHOUR_API AChest : public AActor
{
    GENERATED_BODY()

public:
    AChest();

protected:
    virtual void BeginPlay() override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chest|Components")
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chest|Components")
    UStaticMeshComponent* Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chest|Components")
    UBoxComponent* HitBox;

    // Health
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chest|Health")
    float MaxHP = 50.f;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="Chest|Health")
    float CurrentHP = 0.f;

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp,
               FVector NormalImpulse, const FHitResult& Hit);

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                             class AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(Server, Reliable)
    void ServerBreakChest();

    // Loot
    UPROPERTY(EditAnywhere, Category="Chest|Loot")
    TSubclassOf<AItemPickup> KeyPickupClass;

    UPROPERTY(EditAnywhere, Category="Chest|Loot")
    TSubclassOf<AItemPickup> HealPickupClass;

    UPROPERTY(EditAnywhere, Category="Chest|Loot")
    TSubclassOf<AItemPickup> SpeedPickupClass;

    UPROPERTY(EditAnywhere, Category="Chest|Loot")
    float KeyWeight = 1.f;

    UPROPERTY(EditAnywhere, Category="Chest|Loot")
    float HealWeight = 1.f;

    UPROPERTY(EditAnywhere, Category="Chest|Loot")
    float SpeedWeight = 1.f;

    TSubclassOf<AItemPickup> PickRandomLoot() const;

    // FX/SFX (optional)
    UPROPERTY(EditAnywhere, Category="Chest|FX")
    UParticleSystem* BreakFX = nullptr;

    UPROPERTY(EditAnywhere, Category="Chest|FX")
    USoundBase* BreakSFX = nullptr;

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
