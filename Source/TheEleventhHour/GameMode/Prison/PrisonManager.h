#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PrisonManager.generated.h"

class AEHSurvivorCharacter;
class AInGameGameMode;
class UBoxComponent;

UCLASS()
class THEELEVENTHHOUR_API APrisonManager : public AActor
{
	GENERATED_BODY()

public:
	APrisonManager();

	UPROPERTY(Replicated, BlueprintReadOnly, Category="Prison")
	TArray<AEHSurvivorCharacter*> CapturedSurvivors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prison")
	AActor* EscapeSpot = nullptr;

	UPROPERTY()
	AInGameGameMode* LinkedGameMode = nullptr;

	/* ---------------- Server-side ---------------- */
	void Capture(AEHSurvivorCharacter* Victim);

	/** 🚪 EscapeDoor이 닫힐 때 호출됨 */
	UFUNCTION()
	void ForceMoveCapturedToEscape();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 🔹 블루프린트에서 배치할 콜라이더 (문 앞에 둠) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision", meta=(AllowPrivateAccess="true"))
	UBoxComponent* EscapeTrigger;

	/** 🔹 블루프린트용 이벤트 (플레이어가 트리거 통과 시 호출) */
	UFUNCTION()
	void OnEscapeOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
							  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
							  bool bFromSweep, const FHitResult& SweepResult);

private:
	AActor* FindEscapeSpot();
};
