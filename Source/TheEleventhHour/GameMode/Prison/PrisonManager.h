#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PrisonManager.generated.h"

class AEHSurvivorCharacter;
class AInGameGameMode;

UCLASS()
class THEELEVENTHHOUR_API APrisonManager : public AActor
{
	GENERATED_BODY()

public:
	APrisonManager();

	/** ✅ 현재 수감 중인 생존자들 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Prison")
	TArray<AEHSurvivorCharacter*> CapturedSurvivors;

	/** ✅ 현재 구출 진행자 (한 번에 한 명만) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Rescue")
	AEHSurvivorCharacter* CurrentRescuer = nullptr;

	/** ✅ 구출 진행 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rescue")
	float RescueDuration = 4.0f;

	/** ✅ 탈출 스팟 (없으면 태그 "Spawn.Escape" 탐색) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rescue")
	AActor* EscapeSpot = nullptr;

	/** ✅ GameMode 참조 (BeginPlay에서 자동 연결) */
	UPROPERTY()
	AInGameGameMode* LinkedGameMode = nullptr;

	/* ---------------- Server-side Only ---------------- */
	/** 생존자를 감옥으로 이동 및 등록 */
	void Capture(AEHSurvivorCharacter* Victim);

	/** 구출 시작 (Rescuer → 감옥 근처에서 호출됨) */
	UFUNCTION(Server, Reliable)
	void StartRescue(AEHSurvivorCharacter* Rescuer);

	/** 구출 중단 (Rescuer가 떠나거나 죽었을 때) */
	UFUNCTION(Server, Reliable)
	void CancelRescue(AEHSurvivorCharacter* Rescuer);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	FTimerHandle RescueTimer;

	/** 구출 완료 후 처리 */
	void CompleteRescue();

	/** 모든 수감자 해방 */
	void FreeAllSurvivors();

	/** EscapeSpot 자동 탐색 */
	AActor* FindEscapeSpot();
};
