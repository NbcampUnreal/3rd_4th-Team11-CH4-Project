#include "Survivor/Controller/EHSurvivorController.h"

#include "EnhancedInputSubsystems.h"

AEHSurvivorController::AEHSurvivorController()
: InputMappingContext(nullptr),
  MoveAction(nullptr),
  LookAction(nullptr),
  CrouchAction(nullptr),
  CrossAction(nullptr),
  InteractAction(nullptr)
{
}

void AEHSurvivorController::BeginPlay()
{
  Super::BeginPlay();

  if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
  {
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
    {
      if (InputMappingContext)
      {
        Subsystem->AddMappingContext(InputMappingContext, 0);
      }
    }
  }
}
