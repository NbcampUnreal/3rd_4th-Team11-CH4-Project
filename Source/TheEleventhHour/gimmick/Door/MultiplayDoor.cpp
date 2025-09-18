// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayDoor.h"
#include "Components/BoxComponent.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
// Sets default values
AMultiplayDoor::AMultiplayDoor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
   PrimaryActorTick.bCanEverTick = true;

   SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
   SetRootComponent(SceneComponent);

   BoxComponent = CreateDefaultSubobject<UBoxComponent>("FrontBoxComponent");
   BoxComponent->SetCollisionProfileName(TEXT("Trigger"));
   BoxComponent->SetGenerateOverlapEvents(true);
   BoxComponent->SetupAttachment(RootComponent);
   
   DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>("DoorMesh");
   DoorMesh->SetupAttachment(RootComponent);

   DoorTimeLine =CreateDefaultSubobject<UTimelineComponent>("DoorTimeLine");
}

void AMultiplayDoor::BeginPlay()
{
   Super::BeginPlay();

   if (!OnActorBeginOverlap.IsAlreadyBound(this,&AMultiplayDoor::OnDoorBeginOverlap))
   {
      OnActorBeginOverlap.AddDynamic(this,&AMultiplayDoor::OnDoorBeginOverlap);
   }

   InitialLocalRotation = DoorMesh->GetRelativeRotation();
   TargetLocalRotation = InitialLocalRotation + FRotator(0.0f,OpenYawDelta,0.0f);

   if (DoorTimeLine)
   {
      FOnTimelineFloat Progress;

      Progress.BindUFunction(this,FName("HandleTimelineProgress"));

      if (OpenCurve)
      {
         DoorTimeLine->AddInterpFloat(OpenCurve,Progress);
         DoorTimeLine->SetLooping(false);
      }

      FOnTimelineEvent Finished;
      Finished.BindUFunction(this,FName("HandleFinished"));
      DoorTimeLine->SetTimelineFinishedFunc(Finished);
   }
}

void AMultiplayDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
   Super::EndPlay(EndPlayReason);

   if (OnActorBeginOverlap.IsAlreadyBound(this,&AMultiplayDoor::OnDoorBeginOverlap))
   {
      OnActorBeginOverlap.RemoveDynamic(this,&AMultiplayDoor::OnDoorBeginOverlap);
   }
   
}

void AMultiplayDoor::HandleTimelineProgress(float Alpha)
{
   const FRotator NewRotation = FMath::Lerp(InitialLocalRotation,TargetLocalRotation,Alpha);
   DoorMesh->SetRelativeRotation(NewRotation);
}

void AMultiplayDoor::HandleFinished()
{
   bIsOpening = false;
   bIsOpen = true;
}

void AMultiplayDoor::OnDoorBeginOverlap(AActor* ThisActor, AActor* OtherActor)
{
   if (!OtherActor||OtherActor==this) return;
   if (bIsOpen||bIsOpening) return;

   bIsOpening = true;
   InitialLocalRotation = DoorMesh->GetRelativeRotation();
   TargetLocalRotation = InitialLocalRotation + FRotator(0.0f,OpenYawDelta,0.0f);

   if (DoorTimeLine)
   {
      DoorTimeLine->PlayFromStart();
   }
}

void AMultiplayDoor::Tick(float DeltaTime)
{
   Super::Tick(DeltaTime);
   
}