#include "gimmick/Generators/Generators.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AGenerators::AGenerators()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SetRootComponent(SceneComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	MeshComponent->SetupAttachment(SceneComponent);

	BoxComponent = CreateDefaultSubobject<UBoxComponent>("BoxComponent");
	BoxComponent->SetupAttachment(SceneComponent);
	BoxComponent->SetCollisionProfileName(TEXT("Trigger"));
	BoxComponent->SetGenerateOverlapEvents(true);
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AGenerators::OnGeneratorBeginOverlap);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &AGenerators::OnGeneratorEndOverlap);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>("AudioComponent");
	
	VibrateValue = 0.15f;
	RepairValue = 0.1f;
	RepairProgress = 0.0f;
	RepairDuration = 0.05f;
	RepairCount = 0;
	bIsStartRepairing = false;
	bIsCompleted = false;
	RepairEfficiency = { 0.f, 1.f, 1.8f, 2.4f, 2.8f };

	//Replicate 활성화
	bReplicates = true;
}

void AGenerators::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
}

//복제 대상 목록
void AGenerators::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
	DOREPLIFETIME(AGenerators, RepairProgress);
	DOREPLIFETIME(AGenerators, RepairCount);
	DOREPLIFETIME(AGenerators, bIsStartRepairing);
	DOREPLIFETIME(AGenerators, bIsCompleted);
}

void AGenerators::OnGeneratorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
							 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
							 bool bFromSweep, const FHitResult& SweepResult)
{
	//서버에서만 실행
	if (!HasAuthority()) return;
	
	//액터, 수리 완료 여부, 참여 인원 등 유효성 검사
	if (!OtherActor||OtherActor==this) return;
	if (bIsCompleted) return;
	if (RepairCount >= 4) return;

	//수리 참여 인원 추가, 수리 타이머 핸들이 없을 경우 초기에 한번만 생성
	RepairCount++;
	if (RepairTimerHandle.IsValid()) return;
	GetWorldTimerManager().SetTimer(RepairTimerHandle,this,&AGenerators::RepairGenerator,RepairDuration,true);

	//발전기를 처음 수리하였을 때 true
	if (bIsStartRepairing) return;
	bIsStartRepairing = true;
}

void AGenerators::OnGeneratorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
						   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//서버에서만 실행
	if (!HasAuthority()) return;
	
	//유효성 검사
	if (!OtherActor||OtherActor==this) return;

	//수리 참여 인원 감소
	RepairCount--;
}

void AGenerators::VibrateAnimation()
{
	//진동 좌표 설정
	float RandomX = FMath::FRandRange(0.f,0.1f);
	float RandomY = FMath::FRandRange(0.f,0.1f);
	float RandomZ = FMath::FRandRange(0.f,VibrateValue);

	float NewX = StartLocation.X + RandomX;
	float NewY = StartLocation.Y + RandomY;
	float NewZ = StartLocation.Z + RandomZ;

	FVector VibLocation = FVector(NewX,NewY,NewZ);
	//액터 이동(진동)
	SetActorLocation(VibLocation);
}

void AGenerators::RepairGenerator()
{
	//서버만 해당 로직을 실행함
	if (!HasAuthority()) return;
	
	//진동 애니메이션(MulticastRPC)
	MulticastVibrateAnimation();

	//수리 로직
	RepairProgress += RepairEfficiency[RepairCount] * RepairValue;

	//디버깅 메시지
	MulticastShowRepairProgress();
	
	//수리가 완료될 경우
	if (RepairProgress >= 100.f)
	{
		//수리 완료(MulticastRPC)
		MulticastCompleteGenerator();
	}
}

void AGenerators::CompleteGenerator()
{
	UGameplayStatics::PlaySoundAtLocation(this,CompleteSound,GetActorLocation());
	MulticastStopGeneratorSound();
	GetWorldTimerManager().ClearTimer(RepairTimerHandle);
	bIsCompleted = true;
	
	//추가 수리 완료 로직(카운트 감소or증가 등)
}

void AGenerators::MulticastVibrateAnimation_Implementation()
{
	VibrateAnimation();
}


void AGenerators::MulticastCompleteGenerator_Implementation()
{
	CompleteGenerator();
}

//IsStartRepairing(발전기 동작 여부) 감지하여 사운드 재생
void AGenerators::OnRep_PlayingSound()
{
	if (bIsStartRepairing && !bIsCompleted)
	{
		AudioComponent = UGameplayStatics::SpawnSoundAtLocation(this, GeneratorSound, GetActorLocation());
	}
}

void AGenerators::MulticastStopGeneratorSound_Implementation()
{
	AudioComponent->Stop();
}

//디버깅용
void AGenerators::MulticastShowRepairProgress_Implementation()
{
	if (GEngine)
	{
		FString DebugMsg = FString::Printf(TEXT("RepairProgress: %.2f, RepairCount: %d"), RepairProgress, RepairCount);
		GEngine->AddOnScreenDebugMessage(-1, RepairDuration - 0.01f, FColor::Green, DebugMsg);
	}
}