#include "gimmick/Door/KeypadDoor.h"

#include "EngineUtils.h"
#include "GameMode/InGameGameMode.h"
#include "GameMode/EscapeZone/EscapeZone.h"

AKeypadDoor::AKeypadDoor(): LinkedEscapeZone(nullptr)
{
	bIsRemoteDoor = true;
	bAlwaysRelevant = true;
}

void AKeypadDoor::OpenKeypadDoor()
{
	OpenDoor();

	if (AInGameGameMode* GM = GetWorld()->GetAuthGameMode<AInGameGameMode>())
	{
		GM->OnEscapeDoorOpened();
	}

	// 🔹 월드에 존재하는 모든 EscapeZone을 활성화
	int32 ActivatedCount = 0;
	for (TActorIterator<AEscapeZone> It(GetWorld()); It; ++It)
	{
		if (AEscapeZone* Zone = *It)
		{
			Zone->ActivateEscapeZone();
			ActivatedCount++;
		}
	}
}

