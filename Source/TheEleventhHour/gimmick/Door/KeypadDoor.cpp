#include "gimmick/Door/KeypadDoor.h"

AKeypadDoor::AKeypadDoor()
{
	bIsRemoteDoor = true;
}

void AKeypadDoor::OpenKeypadDoor()
{
	OpenDoor();;
}

