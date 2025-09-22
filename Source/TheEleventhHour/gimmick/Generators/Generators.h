#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Generators.generated.h"

class UStaticMeshComponent;
class UStaticMEshComponent;
class UBoxComponent;
class UAudioComponent;

UCLASS()
class THEELEVENTHHOUR_API AGenerators : public AActor
{
	GENERATED_BODY()
	
public:	
	AGenerators();

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Generators|Components")
	USceneComponent* SceneComponent;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="Generators|Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="Generators|Components")
	UBoxComponent* BoxComponent;

	//발전기, 수리 완료 사운드
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="Generators|Components")
	UAudioComponent* AudioComponent;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Generators|Sounds")
	USoundBase* GeneratorSound;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Generators|Sounds")
	USoundBase* CompleteSound;

	//테스트용 오버랩 이벤트
	UFUNCTION()
	void OnGeneratorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
							 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
							 bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnGeneratorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
						   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	//진동 세기(작동 애니메이션)
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Generators|Properties")
	float VibrateValue;
	//기본 수리 속도
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Generators|Properties")
	float RepairValue;
	//수리 진행도(Replicated 필요)
	UPROPERTY(Replicated, EditAnywhere,BlueprintReadWrite,Category="Generators|Properties")
	float RepairProgress;
	//수리 주기(타이머)
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Generators|Properties")
	float RepairDuration;
	//수리 효율(기본 수리 속도 * 수리 효율)
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Generators|Properties")
	TArray<float> RepairEfficiency;
	//수리 참여 인원(Replicated 필요)
	UPROPERTY(Replicated, EditAnywhere,BlueprintReadWrite,Category="Generators|Properties")
	int32 RepairCount;

	
private:
	//진동 애니메이션
	UFUNCTION(NetMulticast,Unreliable)
	void MulticastVibrateAnimation();
	void VibrateAnimation();
	//발전기 수리
	void RepairGenerator();
	//수리 완료
	UFUNCTION(NetMulticast,Reliable)
	void MulticastCompleteGenerator();
	void CompleteGenerator();
	//발전기 사운드 재생(StartRepairing 변수 감지)
	UFUNCTION()
	void OnRep_PlayingSound();
	//발전기 사운드 중지(Multicast)
	UFUNCTION(NetMulticast,Reliable)
	void MulticastStopGeneratorSound();
	
	//발전기 수리 디버깅
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastShowRepairProgress();
	
	//액터 초기 위치
	FVector StartLocation;
	//초기 작동 여부(Replicated 필요)
	UPROPERTY(ReplicatedUsing=OnRep_PlayingSound)
	bool bIsStartRepairing;
	//수리 완료 여부(Replicated 필요)
	UPROPERTY(Replicated)
	bool bIsCompleted;

	//수리 타이머 핸들
	FTimerHandle RepairTimerHandle;
};
