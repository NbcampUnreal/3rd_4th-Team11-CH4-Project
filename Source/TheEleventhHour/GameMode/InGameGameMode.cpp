#include "GameMode/InGameGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"

AInGameGameMode::AInGameGameMode()
{
	bRolesAssigned = false;
}

void AInGameGameMode::BeginPlay()
{
	Super::BeginPlay();
	bRolesAssigned = false;
}

void AInGameGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	// 여기선 단순히 연결만 처리

	CheckAllPlayersLoaded();
}

void AInGameGameMode::CheckAllPlayersLoaded()
{
	// 서버에서만 실행
	if (!HasAuthority() || bRolesAssigned)
		return;

	const int32 TotalPlayers = GameState ? GameState->PlayerArray.Num() : 0;
	int32 LoadedPlayers = 0;

	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (PS && PS->IsOnlyASpectator() == false)
		{
			++LoadedPlayers;
		}
	}

	// 🔥 모든 플레이어가 로드 완료되었다고 가정 (간단 버전)
	if (LoadedPlayers == TotalPlayers && TotalPlayers > 0)
	{
		AssignRoles();
		bRolesAssigned = true;
	}
}

void AInGameGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	// 모든 플레이어가 들어왔다고 가정하고 게임 시작 직전에 역할 배정
	AssignRoles();
}

void AInGameGameMode::AssignRoles()
{
	TArray<APlayerState*> AllPlayers = GameState->PlayerArray;
	if (AllPlayers.Num() == 0) return;

	// 🔥 랜덤으로 Killer 선택
	int32 KillerIndex = UKismetMathLibrary::RandomInteger(AllPlayers.Num());

	for (int32 i = 0; i < AllPlayers.Num(); ++i)
	{
		if (AInGamePlayerState* PS = Cast<AInGamePlayerState>(AllPlayers[i]))
		{
			PS->bIsKiller = (i == KillerIndex);
			UE_LOG(LogTemp, Log, TEXT("[RoleAssign] %s -> %s"),
				*PS->GetPlayerName(), PS->bIsKiller ? TEXT("KILLER") : TEXT("SURVIVOR"));
		}
	}
}

APawn* AInGameGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	if (AInGamePlayerState* PS = NewPlayer->GetPlayerState<AInGamePlayerState>())
	{
		if (PS->bIsKiller && KillerPawnClass)
		{
			return GetWorld()->SpawnActor<APawn>(KillerPawnClass,
				StartSpot->GetActorLocation(),
				StartSpot->GetActorRotation());
		}
		else if (SurvivorPawnClass)
		{
			return GetWorld()->SpawnActor<APawn>(SurvivorPawnClass,
				StartSpot->GetActorLocation(),
				StartSpot->GetActorRotation());
		}
	}

	return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
}