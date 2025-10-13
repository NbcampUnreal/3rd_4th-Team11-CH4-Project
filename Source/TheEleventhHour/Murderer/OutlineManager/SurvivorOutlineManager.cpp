// Fill out your copyright notice in the Description page of Project Settings.


#include "Murderer/OutlineManager/SurvivorOutlineManager.h"
#include "Survivor/Character/EHSurvivorCharacter.h"
#include "EngineUtils.h"
#include "Murderer/MurdererCharacter.h"

// Called when the game starts or when spawned

ASurvivorOutlineManager::ASurvivorOutlineManager()
{
	PrimaryActorTick.bCanEverTick=false;
}

void ASurvivorOutlineManager::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		UE_LOG(LogTemp,Log,TEXT("SERVER SIDE"));
		return;
	}
	
	if (Cast<AMurdererCharacter>(GEngine->GetFirstLocalPlayerController(GetWorld())->GetPawn()))
	{
		bIsKiller=true;
		
		for (TActorIterator<AEHSurvivorCharacter> It(GetWorld()); It; ++It)
		{
			SurvivorCharacters.Add(*It);
			It->GetMesh()->SetOverlayMaterial(DefaultOutlineMaterial);
			It->OnDestroyed.AddUniqueDynamic(this,&ThisClass::OnActorDestroyed);
		}
		OnActorSpawnedHandle = GetWorld()->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this,&ThisClass::OnActorSpawned));
	}
	
}

void ASurvivorOutlineManager::AfterPossess()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp,Log,TEXT("SERVER SIDE"));
		return;
	}
	
	if (Cast<AMurdererCharacter>(GEngine->GetFirstLocalPlayerController(GetWorld())->GetPawn()))
	{
		bIsKiller=true;
		
		for (TActorIterator<AEHSurvivorCharacter> It(GetWorld()); It; ++It)
		{
			SurvivorCharacters.Add(*It);
			It->GetMesh()->SetOverlayMaterial(DefaultOutlineMaterial);
			It->OnDestroyed.AddUniqueDynamic(this,&ThisClass::OnActorDestroyed);
		}
		OnActorSpawnedHandle = GetWorld()->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this,&ThisClass::OnActorSpawned));
	}
}
void ASurvivorOutlineManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 델리게이트 바인딩 해제해주기
	GetWorld()->RemoveOnActorSpawnedHandler(OnActorSpawnedHandle);

	for (auto a : SurvivorCharacters)
	{
		if (a.IsValid())
			a->OnDestroyed.RemoveAll(this);
	}
}

void ASurvivorOutlineManager::OnActorSpawned(AActor* Actor)
{
	if (bIsKiller)
	{
		if (AEHSurvivorCharacter* Character=Cast<AEHSurvivorCharacter>(Actor))
		{
			SurvivorCharacters.Add(Character);
			Character->GetMesh()->SetOverlayMaterial(DefaultOutlineMaterial);
			Character->OnDestroyed.AddUniqueDynamic(this,&ThisClass::OnActorDestroyed);
		}
	}
}

void ASurvivorOutlineManager::OnActorDestroyed(AActor* DestroyedActor)
{
	if (AEHSurvivorCharacter* Survivor = Cast<AEHSurvivorCharacter>(DestroyedActor))
	{
		SurvivorCharacters.RemoveAll(
			[Survivor](const TWeakObjectPtr<AEHSurvivorCharacter>& Ptr)
			{
				return Ptr.Get() == Survivor;
			}
		);
	}
}
void ASurvivorOutlineManager::SetSurvivorOverlayTransparent()
{
	if (bIsKiller)
	{
		for (auto Survivor : SurvivorCharacters)
		{
			if (Survivor.IsValid())
				Survivor->GetMesh()->SetOverlayMaterial(TransparentOutlineMaterial);
		}
	}
}

void ASurvivorOutlineManager::DisableSurvivorOverlayTransparent()
{
	if (bIsKiller)
	{
		for (auto Survivor : SurvivorCharacters)
		{
			if (Survivor.IsValid())
				Survivor->GetMesh()->SetOverlayMaterial(DefaultOutlineMaterial);
		}
	}
}
