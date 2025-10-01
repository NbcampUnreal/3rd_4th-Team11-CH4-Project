#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MultiplayDoor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UTimelineComponent;
class UCurveFloat;

UCLASS()
class THEELEVENTHHOUR_API AMultiplayDoor : public AActor
{
	GENERATED_BODY()
   
public:   
	AMultiplayDoor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	//기본 컴포넌트
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="Multiplay Door")
	TObjectPtr<USceneComponent> SceneComponent;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Multiplay Door")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	//타임라인 관련
	UPROPERTY()
	TObjectPtr<UTimelineComponent> DoorTimeLine;
	UPROPERTY(EditDefaultsOnly,Category="Multiplay Door|TimeLine")
	TObjectPtr<UCurveFloat> OpenCurve;

	UFUNCTION()
	void HandleTimelineProgress(float Alpha);
	UFUNCTION()
	void OnDoorInterActionFinished();
	UFUNCTION()
	void OpenDoor();
	UFUNCTION()
	void CloseDoor();

	//true일 경우 키패드 연동 문
	bool bIsRemoteDoor;

	//문 자동 폐쇄 타이머
	FTimerHandle DoorCloseTimerHandle;
	
public:
	//캐릭터에서 상호작용 시 호출
	UFUNCTION()
	void InterActionDoor();

	//문 상태 반환
	UFUNCTION()
	bool GetIsOpenDoor() {return bIsOpen;};
	UFUNCTION()
	bool GetIsRemoteDoor() {return bIsRemoteDoor;};
	
	//문이 열리는데 걸리는 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Multiplay Door|Timeline")
	float OpenDuration =0.5f;
	
	//목표 각도(문이 열리는 각도)
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Multiplay Door|Timeline")
	float OpenYawDelta = 90.0f;
	
	//초기 로테이터
	FRotator InitialLocalRotation;
	//목표 로테이터
	FRotator TargetLocalRotation;

	FTimerHandle DoorTimer;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(Replicated)
	bool bIsDoorMoving = false;
	UPROPERTY(ReplicatedUsing = OnRep_DoorState)
	bool bIsOpen = false;
	UFUNCTION()
	void OnRep_DoorState();
};
