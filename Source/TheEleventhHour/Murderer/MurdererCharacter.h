// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "MurdererCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class USkeletalMeshComponent;

UCLASS()
class THEELEVENTHHOUR_API AMurdererCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMurdererCharacter();

	virtual void Tick(float DeltaSeconds) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	//기본 컴포넌트
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="DefaultComponent")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="DefaultComponent")
	TObjectPtr<USkeletalMeshComponent> Mesh1P;
	
	// UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="DefaultComponent")
	// TObjectPtr<USpringArmComponent> SpringArmComponent;
	
	// 인풋 액션
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="InputAction")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="InputAction")
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="InputAction")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="InputAction")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="InputAction")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="InputAction")
	TObjectPtr<UInputAction> VaultAction;
	
	//인풋 액션 바인딩 함수
	void Move(const FInputActionValue& ActionValue);
	void Look(const FInputActionValue& ActionValue);
	void EndPlay(EEndPlayReason::Type EndPlayReason);
	
	UFUNCTION(Server,Reliable)
	void Attack(const FInputActionValue& ActionValue);
	UFUNCTION(Server,Reliable)
	void StartSprint();
	UFUNCTION(Server,Reliable)
	void EndSprint();
	
	//상태 변수, 속성 변수
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category="Variables")
	float BaseSpeed;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category="Variables")
	float SprintSpeed;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category="Variables")
	bool bCanAttack = true;
	UPROPERTY(ReplicatedUsing=OnRep_bIsSprint,VisibleAnywhere,BlueprintReadOnly,Category="Variables")
	bool bIsSprint;

	FTimerHandle SprintStartTimerHandle;
	FTimerHandle SprintEndTimerHandle;
	
	const float MaxSprintGauge=100; //100
	UPROPERTY(Replicated, VisibleAnywhere,BlueprintReadOnly,Category="Variables")
	float SprintGauge;
	
	UFUNCTION()
	void OnRep_bIsSprint();
	
	/* 공격 관련 */
	UFUNCTION(BlueprintCallable, Category="Combat")
	void PerformMeleeHit();

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_PlayAttackAnimMontage();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	UAnimMontage* AttackMontage;
	
	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> HitActorsThisSwing;
	/** 스윙 시작 시 클리어해주고, 노티파이 타이밍마다 판정할 때 사용 */
	UFUNCTION(BlueprintCallable, Category="Combat")
	void BeginMeleeSwingWindow();
	/** 디버그 그리기 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Debug")
	bool bDrawDebug = false;
	/* 전방 스윕 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float MeleeRange = 150.f;
	/* 스윕 구체 반경 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float MeleeRadius = 60.f;
	/* 기본 데미지 값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float MeleeDamage = 25.f;
	/** 데미지 타입 */
	UPROPERTY(EditDefaultsOnly, Category="Combat")
	TSubclassOf<UDamageType> DamageTypeClass;
	/** 충돌 채널 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	
public:	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
};
