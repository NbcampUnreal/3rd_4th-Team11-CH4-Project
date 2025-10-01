// MultiGameModeBase.cpp

#include "GameMode/MultiGameModeBase.h"
#include "Survivor/Controller/EHSurvivorController.h"
#include "gimmick/Keypad/MultiKeypad.h"
#include "EngineUtils.h"

AMultiGameModeBase::AMultiGameModeBase()
	:GeneratorCount(1)
{
}

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

void AMultiGameModeBase::CompletedRepair()
{
	GeneratorCount--;

	if (GeneratorCount <= 0)
	{
		for (TActorIterator<AMultiKeypad> It(GetWorld()); It; ++It)
		{
			AMultiKeypad* Keypad = *It;
			if (Keypad)
			{
				Keypad->SetCanInteract(true);
			}
		}
	}
}

