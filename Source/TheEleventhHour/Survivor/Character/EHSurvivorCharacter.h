#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Survivor/Character/AttributeComponent.h"
#include "EHSurvivorCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

class UInputMappingContext;
class UInputAction;
class APrisonManager;

class UInteractProgressWidget;

struct FInputActionValue;

UCLASS()
class THEELEVENTHHOUR_API AEHSurvivorCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEHSurvivorCharacter();
	
#pragma region Attack
private:
	UFUNCTION(Server, Reliable)
	void ServerRPCAttack();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCAttack();
	UFUNCTION(Server, Reliable)
	void ServerAttackTrace();
	void PlayAttackMontage();

protected:
	UFUNCTION(BlueprintCallable, Category="Input")
	void AttackInput(const FInputActionValue& value);
	UPROPERTY(ReplicatedUsing = OnRep_CanAttack)
	uint8 bCanAttack : 1;

	UFUNCTION()
	void OnRep_CanAttack();
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> AttackMontage;

	float AttackMontagePlayTime;
#pragma endregion

#pragma region Hit&Death
public:
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bIsDead = false;
protected:
	UFUNCTION(NetMulticast, reliable)
	void MulticastHitMontage();
	UFUNCTION(NetMulticast, reliable)
	void MulticastDeathMontage();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* HitReactMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* DeathMontage;
	float DeathMontagePlayTime;

	UFUNCTION()
	void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);
#pragma endregion

#pragma region interaction
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> InteractMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> DoorInteractMontage;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastInteractMontage();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopInteractMontage();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastDoorInteractMontage();

public:
	UFUNCTION(Server,reliable)
	void ServerRPCCancelInteract();
#pragma endregion

#pragma region Sound
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* FootstepSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundAttenuation* FootstepAttenuation;

public:
	UFUNCTION()
	void PlayFootstepSound();
#pragma endregion

protected:
	virtual void BeginPlay() override;

	// ----- 컴포넌트 -----
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
	TObjectPtr<UInputAction> AttackAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> RescueAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
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
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void StartInteraction();
	void EndInteraction(const FInputActionValue& value);
	void Jump() override;
	// --------------------

	// ------ 상태관리 ------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsCrouching;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Replicated)
	bool bIsInteracting;
	UPROPERTY(BlueprintReadWrite)
	bool bIsVaulting;
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
	// ------------------------

	// -------상호작용(발전기, 키패드)--------
	UFUNCTION(Server,reliable)
	void ServerRPCStartInteract();
	UFUNCTION(Server,reliable)
	void ServerRPCInteract();
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SetInteractUI)
	AActor* InteractActor;
	// -----------------------------

	// -------상호작용(UI)--------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> InteractProgressClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UInteractProgressWidget* InteractProgressWidget;
	
	UFUNCTION()
	void OnRep_SetInteractUI();

	UFUNCTION()
	void UpdateProgressUI();
	
	FTimerHandle InteractUIUpdateTimerHandle;
	// -----------------------------

	// --------플레이어 스탯 연동--------
	UFUNCTION(BlueprintCallable)
	void UpdateSurvivorState(ESurvivorState NewState);
	
public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void UseSlot0();
	UFUNCTION()
	void UseSlot1();

	// --------- Item : 서버 로직 -----------
	UFUNCTION(Server, Reliable)
	void ServerRTryOpenDoorWithKey(class AMultiplayDoor* Door);
	UFUNCTION(Server, Reliable)
	void ServerUseConsumableSlot(int32 SlotIndex);

protected:
	UFUNCTION(Server,reliable)
	void ServerRPCInteractionDoorOnPawn(class AMultiplayDoor* Door);

private:
	void Debug_TakeDamage();
	void Debug_PrintHealth() const;

	/* -------------------- 🔹 추가된 감옥 시스템 관련 -------------------- */
private:
	void HandleStateChange(ESurvivorState NewState);  // PlayerState 변경 시 반응
	void TryRescue();                                 // 감옥 구출 시도
	
	UFUNCTION(Server, Reliable)
	void ServerRequestCapture();

	UFUNCTION(Server, Reliable)
	void ServerTryRescue();
	/* --------------------------------------------------------------- */
};
