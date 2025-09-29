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

	UPROPERTY(ReplicatedUsing = OnRep_Attack)
	bool bIsAttack;
	UFUNCTION()
	void OnRep_Attack();

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
		
	void StartInteraction(const FInputActionValue& value);
	void EndInteraction(const FInputActionValue& value);

	void Walk(const FInputActionValue& value);

	UFUNCTION(BlueprintCallable, Category="Combat")
	void AttackInput(const FInputActionValue& value);
	void Attack();
	// --------------------

	// ------ 서버 처리 ------
	UFUNCTION(Server, Reliable)
	void ServerAttack();
	// ----------------------

	// ------ 몽타뉴 설정 ------
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> AttackMontage;
	// ----------------------
	
	// ------ 상태관리 ------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsCrouching;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
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

	float MeleeAttackMontagePlayTime;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	

	// -------데미지 관련--------
	float virtual TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	float MaxHealth=100;
	float Health;
	// ------------------------
	
public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void UseSlot0();

	UFUNCTION()
	void UseSlot1();

// Item : 임시 디버그용 
private:
	void Debug_TakeDamage(); // T 키로 호출
	void Debug_PrintHealth() const; // 현재 체력 출력


//문 네트워킹 관련
protected:
	UFUNCTION(Server,reliable)
	void ServerRPCInteractionDoorOnPawn(class AMultiplayDoor* Door);
	
};
