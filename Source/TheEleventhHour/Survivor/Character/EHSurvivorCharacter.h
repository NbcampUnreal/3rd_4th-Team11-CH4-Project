#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Survivor/Character/AttributeComponent.h"
#include "EHSurvivorCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

class UInputMappingContext;
class UInputAction;

struct FInputActionValue;

UCLASS()
class THEELEVENTHHOUR_API AEHSurvivorCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEHSurvivorCharacter();

#pragma region Attack
	
private:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCAttack();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCAttack();

	UFUNCTION()
	void OnRep_CanAttack();
	void PlayAttackMontage();
	
protected:
	UFUNCTION(BlueprintCallable, Category="Combat")
	void AttackInput(const FInputActionValue& value);
	UPROPERTY(ReplicatedUsing = OnRep_CanAttack)
	uint8 bCanAttack : 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> AttackMontage;

	float AttackMontagePlayTime;
	
#pragma endregion

#pragma region FastRun
	UPROPERTY(ReplicatedUsing = OnRep_CanAttack)
	uint8 bIsRun : 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> FastRunMontage;
#pragma endregion

#pragma region Hit&Death
	UFUNCTION(NetMulticast, reliable)
	void MulticastHitMontage();
	UFUNCTION(NetMulticast, reliable)
	void MulticastDeathMontage();

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bIsDead = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* HitReactMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* DeathMontage;
	float DeathMontagePlayTime;

	UFUNCTION()
	void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);
#pragma endregion

protected:
	virtual void BeginPlay() override;

	// ----- 컴포넌트 ------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UCameraComponent* CameraComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	USpringArmComponent* SpringArm;
	// --------------------

	// ------ 인풋 바인딩 ------
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CrouchAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CrossAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> WalkAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;
	// --------------------
	
	// ------ 스프링암 설정 ------
	FVector DefaultSpringArmLocation;
	FVector CrouchSpringArmLocation;
	
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	// -------------------------
	
	// ------ 동작함수 ------
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void Move(const FInputActionValue& value);
	void Look(const FInputActionValue& value);
	
	void ToggleCrouch(const FInputActionValue& value);
	
	void StartCross(const FInputActionValue& value);
	void EndCross(const FInputActionValue& value);

	UFUNCTION(BlueprintCallable, Category="Interaction")
	void StartInteraction();
	void EndInteraction(const FInputActionValue& value);

	void Walk(const FInputActionValue& value);
	// --------------------
	
	// ------ 상태관리 ------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsCrouching;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Replicated)
	bool bIsInteracting;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsWalking;
	// --------------------

	// ------ 조절 변수 ------
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category ="Variable")
	float InterActionLength;
	// --------------------

	// ------ 상태컴포넌트 ------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Attribute")
	UAttributeComponent* AttributeComp;
	// ------------------------

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// -------데미지 관련--------
	float virtual TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	float MaxHealth=100;
	float Health;
	// ------------------------

	// -------상호작용(발전기, 키패드)--------
	UFUNCTION(Server,reliable)
	void ServerRPCStartInteract();
	UFUNCTION(Server,reliable)
	void ServerRPCCancelInteract();
	UFUNCTION(Server,reliable)
	void ServerRPCInteract();
	
	AActor* InteractActor;
	// -----------------------------
	
public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void UseSlot0();

	UFUNCTION()
	void UseSlot1();

	// --------- Item : 서버 로직 -----------
	// 서버 문 열기
	UFUNCTION(Server, Reliable)
	void ServerRTryOpenDoorWithKey(class AMultiplayDoor* Door);
	// 아이템 사용
	UFUNCTION(Server, Reliable)
	void ServerUseConsumableSlot(int32 SlotIndex);


// Item : 임시 디버그용 
private:
	void Debug_TakeDamage(); // T 키로 호출
	void Debug_PrintHealth() const; // 현재 체력 출력


//문 네트워킹 관련
protected:
	UFUNCTION(Server,reliable)
	void ServerRPCInteractionDoorOnPawn(class AMultiplayDoor* Door);
	
};
