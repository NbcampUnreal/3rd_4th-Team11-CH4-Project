#include "Map/BackgroundMusicActor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

ABackgroundMusicActor::ABackgroundMusicActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// AudioComponent 생성 및 루트 컴포넌트로 설정
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	RootComponent = AudioComponent;
	
	// BeginPlay에서 수동 재생할 예정
	AudioComponent->bAutoActivate = false; 
}

void ABackgroundMusicActor::BeginPlay()
{
	Super::BeginPlay();

	if (BackgroundMusic)
	{
		AudioComponent->SetSound(BackgroundMusic);
		AudioComponent->Play();
	}
}

void ABackgroundMusicActor::PlayBGMSound()
{
	if (BackgroundMusic)
	{
		AudioComponent->Play();
	}
}

void ABackgroundMusicActor::StopBGMSound()
{
	if (BackgroundMusic)
	{
		AudioComponent->Stop();
	}
}

