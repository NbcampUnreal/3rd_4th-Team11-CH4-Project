// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SurvivorOutlineManager.generated.h"

UCLASS()
class THEELEVENTHHOUR_API ASurvivorOutlineManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASurvivorOutlineManager();

	void SetSurvivorOverlayTransparent();
	void DisableSurvivorOverlayTransparent();
	void AfterPossess();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	//델리게이트 바인딩 함수
	void OnActorSpawned(AActor* Actor);
	UFUNCTION()
	void OnActorDestroyed(AActor* DestroyedActor);

	//살인마 클라이언트 여부 캐싱
	bool bIsKiller = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Materials")
	UMaterialInstance* DefaultOutlineMaterial;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Materials")
	UMaterialInstance* TransparentOutlineMaterial;
	
	TArray<TWeakObjectPtr<class AEHSurvivorCharacter>> SurvivorCharacters;
	FDelegateHandle OnActorSpawnedHandle;
	
};
