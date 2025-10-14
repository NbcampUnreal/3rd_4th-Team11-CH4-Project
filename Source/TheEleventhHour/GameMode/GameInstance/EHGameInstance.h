#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "EHGameInstance.generated.h"

// 전방 선언
class UPlayerSaveGame;
class UUserWidget;

UCLASS()
class THEELEVENTHHOUR_API UEHGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UEHGameInstance();

	virtual void Init() override;

	/** IP 직결 연결 */
	void ConnectToIP(const FString& ServerAddress);

	/** 세션 검색 후 자동 참가 */
	void FindSessionsAndJoin();

	// IP 주소를 임시로 저장할 변수
	FString SavedIPAddress;

	UFUNCTION(BlueprintCallable)
	void SetIPAddress(const FString& InIPAddress) { SavedIPAddress = InIPAddress; }

	FString GetIPAddress() const { return SavedIPAddress; }

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

public:

	// [옵션 기능 추가] - Settings 함수들
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SaveGameSettings();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadGameSettings();

	// [옵션 기능 추가] - Setter 함수들
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetMasterVolume(int32 InVolume);

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetGamma(int32 InGamma);

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetMouseSensitivity(int32 InSensitivity);

	// [옵션 기능 추가] - Getter 함수들
	UFUNCTION(BlueprintPure, Category = "Settings")
	int32 GetMasterVolume() const;

	UFUNCTION(BlueprintPure, Category = "Settings")
	int32 GetGamma() const;

	UFUNCTION(BlueprintPure, Category = "Settings")
	int32 GetMouseSensitivity() const;

	// --- UI 관리 기능 ---
	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void ShowOptionsMenu();

	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void HideOptionsMenu();

	UUserWidget* GetOptionsMenuInstance() const { return OptionsMenuInstance; }

protected:

	// --- UI 위젯 클래스 변수 ---
	UPROPERTY(EditDefaultsOnly, Category = "UI Manager|Widget Classes")
	TSubclassOf<UUserWidget> OptionsMenuWidgetClass;

private:
	// [옵션 기능 추가] - 멤버 변수
	UPROPERTY()
	TObjectPtr<class UPlayerSaveGame> CurrentSettings;

	// --- UI 멤버 변수 ---
	UPROPERTY()
	TObjectPtr<UUserWidget> OptionsMenuInstance;
};
