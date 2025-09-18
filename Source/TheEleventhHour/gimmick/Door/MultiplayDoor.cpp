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

   if (!OnActorEndOverlap.IsAlreadyBound(this,&AMultiplayDoor::OnDoorEndOverlap))
   {
      OnActorEndOverlap.AddDynamic(this,&AMultiplayDoor::OnDoorEndOverlap);
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
   }
}

void AMultiplayDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
   Super::EndPlay(EndPlayReason);

   if (OnActorBeginOverlap.IsAlreadyBound(this,&AMultiplayDoor::OnDoorBeginOverlap))
   {
      OnActorBeginOverlap.RemoveDynamic(this,&AMultiplayDoor::OnDoorBeginOverlap);
   }

   if (OnActorEndOverlap.IsAlreadyBound(this,&AMultiplayDoor::OnDoorEndOverlap))
   {
      OnActorEndOverlap.RemoveDynamic(this,&AMultiplayDoor::OnDoorEndOverlap);
   }
   
}

void AMultiplayDoor::HandleTimelineProgress(float Alpha)
{
   const FRotator NewRotation = FMath::Lerp(InitialLocalRotation,TargetLocalRotation,Alpha);
   DoorMesh->SetRelativeRotation(NewRotation);
}

void AMultiplayDoor::OnDoorBeginOverlap(AActor* ThisActor, AActor* OtherActor)
{
   if (!OtherActor||OtherActor==this) return;
   //열려있거나 열리는 도중이면 리턴 (다른 액터가 들어와도 그대로 리턴됨)
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

void AMultiplayDoor::OnDoorEndOverlap(AActor* ThisActor, AActor* OtherActor)
{
   if (!bIsOpen) return;
   
   if (DoorTimeLine)
   {
      DoorTimeLine->ReverseFromEnd();
   }
   bIsOpen=false;
}

void AMultiplayDoor::Tick(float DeltaTime)
{
   Super::Tick(DeltaTime);
   
}
