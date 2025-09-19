#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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

	void Attack(const FInputActionValue& value);
	// --------------------

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
public:	
	virtual void Tick(float DeltaTime) override;

};
