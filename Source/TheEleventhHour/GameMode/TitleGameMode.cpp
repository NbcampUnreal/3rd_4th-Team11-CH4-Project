#include "GameMode/TitleGameMode.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ATitleGameMode::ATitleGameMode()
{
	// 서버가 초기화될 때 사용할 수 있도록 SeamlessTravel 활성화
	bUseSeamlessTravel = true;
}

void ATitleGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && GetNetMode() == NM_DedicatedServer)
	{
		//UE_LOG(LogTemp, Log, TEXT("[Server] TitleGameMode::BeginPlay - Dedicated Server detected. Travelling to WaitingLevel..."));

		// Dedicated 서버가 Title 맵을 기본으로 실행한 뒤 자동으로 WaitingLevel 로 이동
		GetWorld()->ServerTravel(TEXT("/Game/Levels/Maps/WaitingLevel"));
	}
}
