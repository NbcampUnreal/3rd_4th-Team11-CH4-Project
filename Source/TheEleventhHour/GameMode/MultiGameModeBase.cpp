// MultiGameModeBase.cpp

#include "GameMode/MultiGameModeBase.h"
#include "Survivor/Controller/EHSurvivorController.h"

void AMultiGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	AEHSurvivorController* NewPlayerController = Cast<AEHSurvivorController>(NewPlayer);
	if (IsValid(NewPlayerController) == true)
	{
		AlivePlayerControllers.Add(NewPlayerController);
	}
}

void AMultiGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	AEHSurvivorController* ExitingPlayerController = Cast<AEHSurvivorController>(Exiting);
	if (IsValid(ExitingPlayerController) == true && AlivePlayerControllers.Find(ExitingPlayerController) != INDEX_NONE)
	{
		AlivePlayerControllers.Remove(ExitingPlayerController);
		DeadPlayerControllers.Add(ExitingPlayerController);
	}
}
