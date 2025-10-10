#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "EHGameInstance.generated.h"

/** 전용 서버에서 세션 생성 / 클라이언트 세션 검색 및 접속 */
UCLASS()
class THEELEVENTHHOUR_API UEHGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	void FindSessionsAndJoin(); // 클라이언트용

private:
	void HostDedicatedSession();  // 전용 서버용

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	bool bSessionCreated = false;

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
};
