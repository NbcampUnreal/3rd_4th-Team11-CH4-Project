// MultiGameModeBase.cpp

#include "GameMode/MultiGameModeBase.h"
#include "GameFramework/PlayerController.h"

void AMultiGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (IsValid(NewPlayer))
	{
		AlivePlayerControllers.Add(NewPlayer);
		UE_LOG(LogTemp, Log, TEXT("✅ Player Logged In: %s"), *NewPlayer->GetName());
	}
}

void AMultiGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	APlayerController* ExitingPlayer = Cast<APlayerController>(Exiting);
	if (IsValid(ExitingPlayer) && AlivePlayerControllers.Contains(ExitingPlayer))
	{
		AlivePlayerControllers.Remove(ExitingPlayer);
		DeadPlayerControllers.Add(ExitingPlayer);

		UE_LOG(LogTemp, Warning, TEXT("🚪 Player Logged Out: %s"), *ExitingPlayer->GetName());
	}
}
