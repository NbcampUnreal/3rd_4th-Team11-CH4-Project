#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "AttributeComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class THEELEVENTHHOUR_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttributeComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ------체력------
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Attribute")
	float Health;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attribute")
	float MaxHealth = 100.f;
	
	UFUNCTION(BlueprintCallable, Category="Attribute")
	void SetHealth(float SetHealth);
	UFUNCTION(BlueprintCallable, Category="Attribute")
	float GetHealth() const;
	UFUNCTION(BlueprintCallable, Category="Attribute")
	void TakeDamage(float Damage);
	// ----------------

	// ------이동속도------
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Attribute")
	float BaseSpeed = 600.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attribute")
	float SprintSpeed = 900.f;
	UPROPERTY(ReplicatedUsing=OnRep_CurrentSpeed, VisibleAnywhere, BlueprintReadWrite, Category="Attribute")
	float CurrentSpeed;

	UFUNCTION(BlueprintCallable, Category="Attribute")
	void SetCurrentSpeed(float SetSpeed);
	UFUNCTION(BlueprintCallable, Category="Attribute")
	float GetCurrentSpeed() const;

	UFUNCTION()
	void OnRep_CurrentSpeed();
	
	// -------------------

	// Replication 설정
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};