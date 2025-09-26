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
	void HostGame(FName MapName = FName("WaitingLevel"), int32 MaxPlayers = 4, bool bIsLAN = false);

	UFUNCTION(BlueprintCallable)
	void FindAndJoinGame();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUW_TitleLayout> TitleLayoutClass;
	
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	void OnFindSessionsComplete(bool bWasSuccessful);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	
private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FName DesiredMapName;
	UPROPERTY()
	UUW_TitleLayout* TitleLayoutWidget;
};
