// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayDoor.h"
#include "Components/BoxComponent.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
// Sets default values
AMultiplayDoor::AMultiplayDoor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
   PrimaryActorTick.bCanEverTick = false;

   SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
   SetRootComponent(SceneComponent);
   
   DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>("DoorMesh");
   DoorMesh->SetupAttachment(RootComponent);

   DoorTimeLine =CreateDefaultSubobject<UTimelineComponent>("DoorTimeLine");
}

void AMultiplayDoor::BeginPlay()
{
   Super::BeginPlay();
   
   InitialLocalRotation = DoorMesh->GetRelativeRotation();
   TargetLocalRotation = InitialLocalRotation + FRotator(0.0f,OpenYawDelta,0.0f);

   if (DoorTimeLine)
   {
      FOnTimelineFloat Progress;

      Progress.BindDynamic(this,&AMultiplayDoor::HandleTimelineProgress);

      if (OpenCurve)
      {
         DoorTimeLine->AddInterpFloat(OpenCurve,Progress);
         DoorTimeLine->SetLooping(false);
      }
   }
}

void AMultiplayDoor::HandleTimelineProgress(float Alpha)
{
   const FRotator NewRotation = FMath::Lerp(InitialLocalRotation,TargetLocalRotation,Alpha);
   DoorMesh->SetRelativeRotation(NewRotation);
}

void AMultiplayDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
   Super::EndPlay(EndPlayReason);
}

void AMultiplayDoor::OpenDoor()
{
   if (bIsOpen||bIsOpening) return;

   bIsOpening = true;
   TargetLocalRotation = InitialLocalRotation + FRotator(0.0f,OpenYawDelta,0.0f);

   if (DoorTimeLine)
   {
      DoorTimeLine->SetPlayRate(1/OpenDuration);
      DoorTimeLine->PlayFromStart();
   }
   
   bIsOpen = true;
   bIsOpening = false;
}

void AMultiplayDoor::CloseDoor()
{
   if (!bIsOpen) return;
   
   if (DoorTimeLine)
   {
      DoorTimeLine->ReverseFromEnd();
   }
   bIsOpen=false;
}

void AMultiplayDoor::InterActionDoor()
{
   if (bIsOpening) return;
   
   if (!bIsOpen)
      OpenDoor();
   else
      CloseDoor();
}
