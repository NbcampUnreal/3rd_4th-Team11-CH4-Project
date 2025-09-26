#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "TitlePlayerController.generated.h"

class UUW_TitleLayout;

UCLASS()
class THEELEVENTHHOUR_API ATitlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	void HostGame(FName MapName = FName("Map_Game"), int32 MaxPlayers = 4, bool bIsLAN = false);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUW_TitleLayout> TitleLayoutClass;
	
private:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	FName DesiredMapName;

	UPROPERTY()
	UUW_TitleLayout* TitleLayoutWidget;
};
