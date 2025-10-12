#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "EHGameInstance.generated.h"

/**
 * 세션 생성/검색/접속 및 Dedicated 서버 관리
 */
UCLASS()
class THEELEVENTHHOUR_API UEHGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	/** IP 직결 연결 */
	void ConnectToIP(const FString& ServerAddress);

	/** 세션 검색 후 자동 참가 */
	void FindSessionsAndJoin();

protected:
	/** 서버 전용 Dedicated 세션 호스트 */
	void HostDedicatedSession();

	// Delegates
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	bool bSessionCreated = false;
};
