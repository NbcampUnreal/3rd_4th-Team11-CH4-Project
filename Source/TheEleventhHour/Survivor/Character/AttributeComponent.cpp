#include "AttributeComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h" // Item : 이동속도 증가용 타이머

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

// ------속도------
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
			//UE_LOG(LogTemp, Warning, TEXT("CurrentSpeed 적용: %f"), CurrentSpeed);
		}
	}
}
// --------------

// ------레플리케이션------
void UAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UAttributeComponent, Health);
	DOREPLIFETIME(UAttributeComponent, CurrentSpeed);
}
// ---------------------

// Item : 임시 디버그용
void UAttributeComponent::ApplyDamage(float Amount)
{
	if (Amount <= 0.f) return;
	SetHealth(Health - Amount);
}

void UAttributeComponent::ApplyHeal(float Amount)
{
	if (Amount <= 0.f) return;
	SetHealth(Health + Amount);
}
// Item 이동속도 증가
void UAttributeComponent::ApplySpeedBuff(float AddSpeed, float Duration)
{
	// 현재속도를 올리고, Duration 이후에 원복
	const float Target = CurrentSpeed + AddSpeed;  // CurrentSpeed/OnRep는 기존 구조 재사용
	SetCurrentSpeed(Target);                       // 이미 MaxWalkSpeed에 반영되도록 구성되어 있음

	// 기존 타이머가 있다면 갱신
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle_SpeedBuff);
		World->GetTimerManager().SetTimer(
			TimerHandle_SpeedBuff,
			[this]()
			{
				// 원복: BaseSpeed로 복귀
				SetCurrentSpeed(BaseSpeed);
			},
			Duration, false
		);
	}
}
// --------Item end--------