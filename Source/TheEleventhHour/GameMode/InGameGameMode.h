#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InGameGameMode.generated.h"

class AInGamePlayerState;

/**
 * 인게임 모드
 * - WaitingLevel에서 넘어온 플레이어들의 역할을 Killer / Survivor 로 나누어 Pawn 스폰
 * - SeamlessTravel 후 PlayerState 유지 및 재포제스 처리
 */
UCLASS()
class THEELEVENTHHOUR_API AInGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AInGameGameMode();

protected:
	/** 🔹 WaitingLevel → GameLevel 이동 완료 시 호출 (PlayerState 유지됨) */
	virtual void PostSeamlessTravel() override;

	/** 🔹 Killer / Survivor 역할 부여 */
	void AssignRoles();

	/** 🔹 이미 잘못 스폰된 Killer만 교체해서 올바른 Pawn으로 재포제스 */
	void RespawnKillerIfNeeded();

	/** 🔹 Pawn 스폰 시 역할에 맞는 PlayerStart를 찾아서 스폰 */
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;

	/** 첫 역할 부여 완료 여부 (중복 할당 방지) */
	bool bRolesAssigned = false;

public:
	/** Killer로 사용할 Pawn 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	TSubclassOf<APawn> KillerPawnClass;

	/** Survivor로 사용할 Pawn 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	TSubclassOf<APawn> SurvivorPawnClass;
};
