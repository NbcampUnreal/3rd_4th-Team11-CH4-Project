#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EscapeZone.generated.h"

class UBoxComponent;

/**
 * 🔹 최종 탈출 구역 클래스
 * - 발전기 전부 수리 → 키패드 문 오픈 → 진입 시 탈출 판정
 * - 킬러는 무시
 * - 문이 닫혀 있으면 탈출 판정 비활성화
 */
UCLASS()
class THEELEVENTHHOUR_API AEscapeZone : public AActor
{
	GENERATED_BODY()

public:
	AEscapeZone();

protected:
	virtual void BeginPlay() override;

	/** 탈출 감지용 박스 콜라이더 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Escape")
	UBoxComponent* EscapeTrigger;

	/** 디버그용 Mesh (편집기에서 위치 확인용) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Escape")
	UStaticMeshComponent* DebugMesh;

	/** 🔹 현재 탈출 가능 여부 (문 열림 여부) */
	UPROPERTY(BlueprintReadOnly, Category="Escape")
	bool bEscapeEnabled = false;

public:
	/** 🔹 문이 열릴 때 호출 (GameMode나 Door Blueprint에서) */
	UFUNCTION(BlueprintCallable, Category="Escape")
	void ActivateEscapeZone();

	/** 🔹 문이 닫힐 때 호출 (선택사항) */
	UFUNCTION(BlueprintCallable, Category="Escape")
	void DeactivateEscapeZone();

protected:
	/** 🔹 Overlap 이벤트 */
	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};
