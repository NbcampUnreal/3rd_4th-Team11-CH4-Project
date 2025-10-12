#include "gimmick/Keypad/MultiKeypad.h"
#include "gimmick/Door/KeypadDoor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

AMultiKeypad::AMultiKeypad()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SetRootComponent(SceneComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	MeshComponent->SetupAttachment(SceneComponent);

	BoxComponent = CreateDefaultSubobject<UBoxComponent>("BoxComponent");
	BoxComponent->SetupAttachment(SceneComponent);
	BoxComponent->SetCollisionProfileName(TEXT("Trigger"));

	AudioComponent = CreateDefaultSubobject<UAudioComponent>("AudioComponent");
	
	ProgressValue = 0.22f;
	Progress = 0.0f;
	ProgressDuration = 0.05f;
	bIsProgressing = false;
	bIsCompleted = false;
	bIsCanInteract = false;

	//Replicate 활성화
	bReplicates = true;
}

void AMultiKeypad::BeginPlay()
{
	Super::BeginPlay();
}

//복제 대상 목록
void AMultiKeypad::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
	DOREPLIFETIME(AMultiKeypad, Progress);
	DOREPLIFETIME(AMultiKeypad, bIsProgressing);
	DOREPLIFETIME(AMultiKeypad, bIsCompleted);
	DOREPLIFETIME(AMultiKeypad, bIsCanInteract);
}

void AMultiKeypad::StartProgress()
{
	//발전기가 다 수리되어야 상호작용 가능
	if (!bIsCanInteract) return;
	//서버에서만 실행
	if (!HasAuthority()) return;
	
	//예외 처리
	if (bIsCompleted) return;
	if (bIsProgressing) return;

	bIsProgressing = true;
	
	//타이머 핸들이 없을 경우 초기에 한번만 생성
	if (ProgressTimerHandle.IsValid()) return;
	GetWorldTimerManager().SetTimer(ProgressTimerHandle,this,&AMultiKeypad::ProgressKeypad,ProgressDuration,true);
}

void AMultiKeypad::StopProgress()
{
	//서버에서만 실행
	if (!HasAuthority()) return;

	//false 처리
	bIsProgressing = false;

	MulticastRPCStopPlaySound();
	GetWorldTimerManager().ClearTimer(ProgressTimerHandle);
}

void AMultiKeypad::ProgressKeypad()
{
	//서버만 해당 로직을 실행함
	if (!HasAuthority()) return;

	//키패드 입력 로직
	Progress += ProgressValue;

	//디버깅 메시지
	MulticastRPCShowProgress();
	
	//입력이 완료될 경우
	if (Progress >= 100.f)
	{
		MulticastRPCCompleteProgress();
	}
}

void AMultiKeypad::CompleteProgress()
{
	UGameplayStatics::PlaySoundAtLocation(this,CompleteSound,GetActorLocation());
	GetWorldTimerManager().ClearTimer(ProgressTimerHandle);
	bIsCompleted = true;
	bIsProgressing = false;
	MulticastRPCStopPlaySound();
	OpenNearestDoor();
}

void AMultiKeypad::MulticastRPCCompleteProgress_Implementation()
{
	CompleteProgress();
}

//IsProgressing(입력 여부) 감지하여 사운드 재생
void AMultiKeypad::OnRep_PlaySound()
{
	UE_LOG(LogTemp, Warning, TEXT("OnRep_PlaySound called: bIsProgressing = %s"), bIsProgressing ? TEXT("true") : TEXT("false"));
	if (bIsProgressing && !bIsCompleted)
	{
		AudioComponent = UGameplayStatics::SpawnSoundAtLocation(this, ProgressSound, GetActorLocation());
	}
}

void AMultiKeypad::MulticastRPCStopPlaySound_Implementation()
{
	if (AudioComponent)
	{
		AudioComponent->Stop();
	}
}

void AMultiKeypad::OpenNearestDoor()
{
	float NearestDistance = FLT_MAX;
	AKeypadDoor* NearestDoor = nullptr;

	for (TActorIterator<AKeypadDoor> It(GetWorld()); It; ++It)
	{
		AKeypadDoor* Door = *It;
		float Distance = FVector::Dist(Door->GetActorLocation(), GetActorLocation());

		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			NearestDoor = Door;
		}
	}

	if (!NearestDoor) return;
	NearestDoor->OpenKeypadDoor();
}


//디버깅용
void AMultiKeypad::MulticastRPCShowProgress_Implementation()
{
	if (GEngine)
	{
		FString DebugMsg = FString::Printf(TEXT("KeypadProgress: %.2f"), Progress);
		GEngine->AddOnScreenDebugMessage(-1, ProgressDuration - 0.01f, FColor::Blue, DebugMsg);
	}
}

void AMultiKeypad::SetCanInteract(bool CanInteract)
{
	bIsCanInteract = CanInteract;
	MulticastRPCShowKeypad();
}

void AMultiKeypad::MulticastRPCShowKeypad_Implementation()
{
	if (MeshComponent)
	{
		MeshComponent->SetRenderCustomDepth(true);
		MeshComponent->SetCustomDepthStencilValue(1);
	}
	UGameplayStatics::PlaySoundAtLocation(this,SirenSound,GetActorLocation());
}