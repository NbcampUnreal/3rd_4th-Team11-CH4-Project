#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EHSurvivorController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class THEELEVENTHHOUR_API AEHSurvivorController : public APlayerController
{
	GENERATED_BODY()

public:
	AEHSurvivorController();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CrouchAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CrossAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	virtual void BeginPlay() override;
};
