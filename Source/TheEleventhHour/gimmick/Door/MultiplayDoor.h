// Fill out your copyright notice in the Description page of Project Settings.

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
	// Sets default values for this actor's properties
	AMultiplayDoor();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleTimelineProgress(float Alpha);
	
	
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
	void OpenDoor();

	UFUNCTION()
	void CloseDoor();
	
public:
	
	UFUNCTION()
	void InterActionDoor();
	
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
	
	bool bIsOpening = false;
	bool bIsOpen = false;
	
};
