// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MurdererCharacter.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputComponent;

UCLASS()
class THEELEVENTHHOUR_API AMurdererCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMurdererCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//기본 컴포넌트
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="DefaultComponent")
	TObjectPtr<USceneComponent> SceneComponent;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="DefaultComponent")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="DefaultComponent")
	TObjectPtr<UCameraComponent> CameraComponent;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="DefaultComponent")
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	
	// 인풋 액션
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="InputAction")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="InputAction")
	TObjectPtr<UInputComponent> MoveAction;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="InputAction")
	TObjectPtr<UInputComponent> LookAction;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="InputAction")
	TObjectPtr<UInputComponent> JumpAction;
	
	//상태 변수, 속성 변수
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Variables")
	float BaseSpeed;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Variables")
	bool bCanAttack = true;

public:	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
};
