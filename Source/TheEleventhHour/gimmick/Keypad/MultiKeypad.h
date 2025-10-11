#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MultiKeypad.generated.h"

class UStaticMeshComponent;
class UStaticMEshComponent;
class UBoxComponent;
class UAudioComponent;

UCLASS()
class THEELEVENTHHOUR_API AMultiKeypad : public AActor
{
	GENERATED_BODY()
	
public:	
	AMultiKeypad();
	
	void StartProgress();
	void StopProgress();
	void SetCanInteract(bool CanInteract);
	
protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Generators|Components")
	USceneComponent* SceneComponent;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="Generators|Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="Generators|Components")
	UBoxComponent* BoxComponent;

	//키패드 사운드
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="Generators|Components")
	UAudioComponent* AudioComponent;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Generators|Sounds")
	USoundBase* ProgressSound;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Generators|Sounds")
	USoundBase* CompleteSound;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Generators|Sounds")
	USoundBase* SirenSound;
	
	//기본 진척 속도
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Generators|Properties")
	float ProgressValue;
	//진척도(Replicated 필요)
	UPROPERTY(Replicated, EditAnywhere,BlueprintReadWrite,Category="Generators|Properties")
	float Progress;
	//타이머 주기
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Generators|Properties")
	float ProgressDuration;

	UPROPERTY(Replicated)
	bool bIsCanInteract;

private:
	UFUNCTION(NetMulticast,Reliable)
	void MulticastRPCShowKeypad();
	//키패드 입력
	void ProgressKeypad();
	//입력 완료
	UFUNCTION(NetMulticast,Reliable)
	void MulticastRPCCompleteProgress();
	void CompleteProgress();
	//사운드 재생(IsProgressing 변수 감지), 정지
	UFUNCTION()
	void OnRep_PlaySound();
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCStopPlaySound();
	//디버깅
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCShowProgress();
	//문 개방
	UFUNCTION()
	void OpenNearestDoor();
	
	//입력 진행 여부(Replicated 필요)
	UPROPERTY(ReplicatedUsing=OnRep_PlaySound)
	bool bIsProgressing;
	//입력 완료 여부(Replicated 필요)
	UPROPERTY(Replicated)
	bool bIsCompleted;

	//키패드 타이머 핸들
	FTimerHandle ProgressTimerHandle;
};
