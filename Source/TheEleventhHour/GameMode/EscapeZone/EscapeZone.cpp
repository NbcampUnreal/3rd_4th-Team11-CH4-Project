#include "GameMode/EscapeZone/EscapeZone.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameMode/GameState/InGameGameState.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
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
	UE_LOG(LogTemp, Log, TEXT("[EscapeZone] Initialized. Waiting for door activation."));
}

/* 🔹 문 열릴 때 활성화 */
void AEscapeZone::ActivateEscapeZone()
{
	bEscapeEnabled = true;
	UE_LOG(LogTemp, Warning, TEXT("[EscapeZone] Escape Enabled — Survivors can now escape."));
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
		PS->ServerSetSurvivorState(ESurvivorState::Escaped);
		UE_LOG(LogTemp, Warning, TEXT("[EscapeZone] %s successfully escaped the game!"), *Survivor->GetName());

		// 🔹 탈출 후 연출 (움직임 제한)
		Survivor->DisableInput(nullptr);
		Survivor->GetCharacterMovement()->DisableMovement();

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1, 3.f, FColor::Green,
				FString::Printf(TEXT("✅ %s ESCAPED!"), *Survivor->GetName())
			);
		}
	}

	// after marking escaped...
	APlayerController* PC = Cast<APlayerController>(Survivor->GetController());
	if (PC)
	{
		// 1) Find a target survivor pawn to spectate (simple example: first non-escaped, non-killer)
		APawn* SpectateTarget = nullptr;
		if (AGameStateBase* GS = GetWorld()->GetGameState())
		{
			for (APlayerState* PSIter : GS->PlayerArray)
			{
				if (AInGamePlayerState* IGPS = Cast<AInGamePlayerState>(PSIter))
				{
					if (IGPS->bIsKiller) continue;
					if (IGPS->SurvivorState == ESurvivorState::Escaped) continue;
					if (IGPS->SurvivorState == ESurvivorState::Dead) continue;

					// get their pawn
					if (AController* OtherController = Cast<AController>(IGPS->GetOwner()))
					{
						if (APawn* OtherPawn = OtherController->GetPawn())
						{
							SpectateTarget = OtherPawn;
							break;
						}
					}
				}
			}
		}

		// 2) If we found a live target, set view target
		if (SpectateTarget)
		{
			// Unpossess optional: PC->UnPossess();
			// Blend view to target (server-side call will replicate view to that client)
			PC->SetViewTargetWithBlend(SpectateTarget, 0.6f, EViewTargetBlendFunction::VTBlend_Cubic);

			UE_LOG(LogTemp, Log, TEXT("[EscapeZone] %s will spectate %s"),
				*PC->GetName(), *GetNameSafe(SpectateTarget));
		}
		else
		{
			// Optional: no live survivors -> spawn/possess spectator pawn or freecam
			UE_LOG(LogTemp, Log, TEXT("[EscapeZone] No valid spectate target found; consider spawning spectator pawn."));
		}
	}

}
