#include "GameMode/EscapeZone/EscapeZone.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameMode/GameState/InGameGameState.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "GameMode/InGameGameMode.h"
#include "Survivor/Character/EHSurvivorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AEscapeZone::AEscapeZone()
{
	PrimaryActorTick.bCanEverTick = false;

	// ✅ 콜라이더 생성
	EscapeTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("EscapeTrigger"));
	RootComponent = EscapeTrigger;
	EscapeTrigger->SetBoxExtent(FVector(300.f, 300.f, 200.f));
	EscapeTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EscapeTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	EscapeTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	EscapeTrigger->SetGenerateOverlapEvents(true);

	// ✅ 디버그용 Mesh (에디터에서만 보이게)
	DebugMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugMesh"));
	DebugMesh->SetupAttachment(RootComponent);
	DebugMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DebugMesh->SetHiddenInGame(true);

	// ✅ 이벤트 바인딩
	EscapeTrigger->OnComponentBeginOverlap.AddDynamic(this, &AEscapeZone::OnOverlapBegin);
}

void AEscapeZone::BeginPlay()
{
	Super::BeginPlay();

	bEscapeEnabled = false; // 기본적으로 문 열리기 전엔 비활성화
}

/* 🔹 문 열릴 때 활성화 */
void AEscapeZone::ActivateEscapeZone()
{
	bEscapeEnabled = true;
}

/* 🔹 문 닫힐 때 비활성화 (선택) */
void AEscapeZone::DeactivateEscapeZone()
{
	bEscapeEnabled = false;
	UE_LOG(LogTemp, Warning, TEXT("[EscapeZone] Escape Disabled — Door closed."));
}

/* 🔹 탈출 판정 */
void AEscapeZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	AEHSurvivorCharacter* Survivor = Cast<AEHSurvivorCharacter>(OtherActor);
	if (!Survivor) return;

	AInGamePlayerState* PS = Survivor->GetPlayerState<AInGamePlayerState>();
	if (!PS || PS->bIsKiller) return;

	// 🚫 문이 아직 안 열렸으면 탈출 불가
	if (!bEscapeEnabled)
	{
		UE_LOG(LogTemp, Log, TEXT("[EscapeZone] %s tried to escape, but door is closed."), *Survivor->GetName());
		return;
	}

	// ✅ 실제 탈출 처리
	if (PS->SurvivorState != ESurvivorState::Escaped)
	{
		// 🔸 상태 갱신
		PS->ServerSetSurvivorState(ESurvivorState::Escaped);
		UE_LOG(LogTemp, Warning, TEXT("[EscapeZone] %s successfully escaped the game!"), *Survivor->GetName());

		// 🔸 기존 기능 유지 (입력 제한, 움직임 정지)
		Survivor->DisableInput(nullptr);
		Survivor->GetCharacterMovement()->DisableMovement();
	}

	// 🔹 GameMode에서 Pawn 제거 + 관전자 모드 진입 처리
	if (AInGameGameMode* GM = Survivor->GetWorld()->GetAuthGameMode<AInGameGameMode>())
	{
		AController* PlayerController = Survivor->GetController();
		if (PlayerController)
		{
			GM->HandlePlayerRemoved(PlayerController, TEXT("You escaped successfully!"));
		}
	}
}
