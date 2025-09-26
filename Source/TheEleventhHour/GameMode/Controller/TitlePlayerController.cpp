#include "GameMode/Controller/TitlePlayerController.h"
#include "GameMode/UI/UW_TitleLayout.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

void ATitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (TitleLayoutClass)
	{
		TitleLayoutWidget = CreateWidget<UUW_TitleLayout>(this, TitleLayoutClass);
		if (TitleLayoutWidget)
		{
			TitleLayoutWidget->AddToViewport();
		}
	}
}

void ATitlePlayerController::HostGame(FName MapName, int32 MaxPlayers, bool bIsLAN)
{
	DesiredMapName = MapName;

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			// 기존 세션이 있다면 제거
			if (SessionInterface->GetNamedSession(NAME_GameSession))
			{
				SessionInterface->DestroySession(NAME_GameSession);
			}

			// 세션 설정
			FOnlineSessionSettings SessionSettings;
			SessionSettings.NumPublicConnections = MaxPlayers;
			SessionSettings.bIsLANMatch = bIsLAN;
			SessionSettings.bUsesPresence = true;      // 로비/친구 초대 가능
			SessionSettings.bShouldAdvertise = true;   // 검색 가능하게
			SessionSettings.bAllowJoinInProgress = true;

			SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
				FOnCreateSessionCompleteDelegate::CreateUObject(this, &ATitlePlayerController::OnCreateSessionComplete)
			);

			// 세션 생성
			SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
		}
	}
}

void ATitlePlayerController::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		// Listen 서버로 맵 열기
		FString TravelURL = DesiredMapName.ToString() + TEXT("?listen");
		UGameplayStatics::OpenLevel(GetWorld(), *TravelURL, true);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("세션 생성 실패"));
	}
}