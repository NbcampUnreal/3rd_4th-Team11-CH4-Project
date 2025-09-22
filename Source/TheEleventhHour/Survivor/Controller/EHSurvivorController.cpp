#include "Survivor/Controller/EHSurvivorController.h"

#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"

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

  if (IsLocalController() == false)
  {
    return;
  }

  FInputModeGameOnly GameOnly;
  SetInputMode(GameOnly);
  
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
