#include "Survivor/Character/EHNotify.h"
#include "Survivor/Character/EHSurvivorCharacter.h"

void UEHNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (MeshComp)
    {
        AEHSurvivorCharacter* Character = Cast<AEHSurvivorCharacter>(MeshComp->GetOwner());
        if (Character)
        {
            Character->PlayFootstepSound();
        }
    }
}