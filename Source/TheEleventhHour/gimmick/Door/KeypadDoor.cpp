#include "gimmick/Door/KeypadDoor.h"

AKeypadDoor::AKeypadDoor()
{
	bIsRemoteDoor = true;
	bAlwaysRelevant = true;
}

void AKeypadDoor::OpenKeypadDoor()
{
	OpenDoor();;
}

