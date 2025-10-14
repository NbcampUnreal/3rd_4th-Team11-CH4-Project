#include "Survivor/OutlineManager/SurvivorVisibilityManager.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Survivor/Character/EHSurvivorCharacter.h"

ASurvivorVisibilityManager::ASurvivorVisibilityManager()
{
	PrimaryActorTick.bCanEverTick = false;

}

void ASurvivorVisibilityManager::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어가 생존자인 경우에만 적용
	if (Cast<AEHSurvivorCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)))
	{
		bIsLocalSurvivor = true;

		for (TActorIterator<AEHSurvivorCharacter> It(GetWorld()); It; ++It)
		{
			AEHSurvivorCharacter* Survivor = *It;
			APawn* LocalPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

			if (Survivor && Survivor != LocalPawn) // 본인 제외
			{
				SurvivorCharacters.Add(Survivor);
				Survivor->GetMesh()->SetOverlayMaterial(DefaultOutlineMaterial);
			}
		}

		// 이후 스폰되는 생존자들도 자동 처리
		OnActorSpawnedHandle = GetWorld()->AddOnActorSpawnedHandler(
			FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::OnActorSpawned));
	}
}

void ASurvivorVisibilityManager::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (GetWorld())
	{
		GetWorld()->RemoveOnActorSpawnedHandler(OnActorSpawnedHandle);
	}

	for (auto Survivor : SurvivorCharacters)
	{
		if (Survivor.IsValid())
			Survivor->OnDestroyed.RemoveAll(this);
	}
}

void ASurvivorVisibilityManager::OnActorSpawned(AActor* Actor)
{
	if (!bIsLocalSurvivor) return;

	if (AEHSurvivorCharacter* Survivor = Cast<AEHSurvivorCharacter>(Actor))
	{
		APawn* LocalPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (Survivor != LocalPawn)
		{
			SurvivorCharacters.Add(Survivor);
			Survivor->GetMesh()->SetOverlayMaterial(DefaultOutlineMaterial);
		}
	}
}

void ASurvivorVisibilityManager::OnActorDestroyed(AActor* DestroyedActor)
{
	if (AEHSurvivorCharacter* Survivor = Cast<AEHSurvivorCharacter>(DestroyedActor))
	{
		SurvivorCharacters.RemoveAll(
			[Survivor](const TWeakObjectPtr<AEHSurvivorCharacter>& Ptr)
			{
				return Ptr.Get() == Survivor;
			});
	}
}
