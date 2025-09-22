#include "AttributeComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	Health = MaxHealth;

	CurrentSpeed = BaseSpeed;
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	Health = MaxHealth;

	OnRep_CurrentSpeed();
}

// ------체력------
void UAttributeComponent::SetHealth(float SetHealth)
{
	Health = FMath::Clamp(SetHealth, 0.f, MaxHealth);
}

float UAttributeComponent::GetHealth() const
{
	return Health;
}

void UAttributeComponent::TakeDamage(float Damage)
{
	SetHealth(Health - Damage);
}
// --------------

void UAttributeComponent::SetCurrentSpeed(float SetSpeed)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		CurrentSpeed = SetSpeed;
		OnRep_CurrentSpeed();
	}
}

float UAttributeComponent::GetCurrentSpeed() const
{
	return CurrentSpeed;
}

void UAttributeComponent::OnRep_CurrentSpeed()
{
	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		if (Char->GetCharacterMovement())
		{
			Char->GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;
			UE_LOG(LogTemp, Warning, TEXT("CurrentSpeed 적용: %f"), CurrentSpeed);
		}
	}
}

// ------레플리케이션------
void UAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UAttributeComponent, Health);
	DOREPLIFETIME(UAttributeComponent, CurrentSpeed);
}
// ---------------------
