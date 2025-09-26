#include "GameMode/Controller/TitlePlayerController.h"
#include "GameMode/UI/UW_TitleLayout.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

void ATitlePlayerController::BeginPlay()
{
    Super::BeginPlay();

    bShowMouseCursor = true;
    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);
    
    // 세션 인터페이스 가져오기
    if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
    {
        SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
                FOnCreateSessionCompleteDelegate::CreateUObject(this, &ATitlePlayerController::OnCreateSessionComplete)
            );
            SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
                FOnFindSessionsCompleteDelegate::CreateUObject(this, &ATitlePlayerController::OnFindSessionsComplete)
            );
            SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
                FOnJoinSessionCompleteDelegate::CreateUObject(this, &ATitlePlayerController::OnJoinSessionComplete)
            );
        }
    }

    // UI 생성
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

    if (!SessionInterface.IsValid())
        return;

    // 기존 세션 제거
    if (SessionInterface->GetNamedSession(NAME_GameSession))
    {
        SessionInterface->DestroySession(NAME_GameSession);
    }

    // 세션 설정
    FOnlineSessionSettings SessionSettings;
    SessionSettings.NumPublicConnections = MaxPlayers;
    SessionSettings.bIsLANMatch = bIsLAN;
    SessionSettings.bShouldAdvertise = true;    // 검색 가능
    SessionSettings.bAllowJoinInProgress = true;
    SessionSettings.bUsesPresence = !bIsLAN;    // 온라인 플랫폼일 때 Presence 사용
    SessionSettings.bUseLobbiesIfAvailable = true;

    SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
}

void ATitlePlayerController::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("[Host] Session created successfully"));
        UGameplayStatics::OpenLevel(GetWorld(), *DesiredMapName.ToString(), true, "listen");
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Host] Session creation failed"));
    }
}

void ATitlePlayerController::FindAndJoinGame()
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[Client] SessionInterface is invalid!"));
        return;
    }

    const ULocalPlayer* LocalPlayer = GetWorld() ? GetWorld()->GetFirstLocalPlayerFromController() : nullptr;
    if (!LocalPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("[Client] LocalPlayer is null - cannot search sessions"));
        return;
    }

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->MaxSearchResults = 10;
    SessionSearch->bIsLanQuery = true;  // LAN 테스트용

    if (!SessionSearch->bIsLanQuery)
    {
        UE_LOG(LogTemp, Log, TEXT("[Client] Using online search with Presence filter"));
        SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[Client] Using LAN search (Presence disabled)"));
    }

    bool bStarted = SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
    if (bStarted)
    {
        UE_LOG(LogTemp, Log, TEXT("[Client] FindSessions started successfully..."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Client] FindSessions failed to start!"));
    }
}

void ATitlePlayerController::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[Client] SessionInterface is invalid in OnFindSessionsComplete"));
        return;
    }

    if (!bWasSuccessful)
    {
        UE_LOG(LogTemp, Error, TEXT("[Client] FindSessions failed (bWasSuccessful = false)"));
        return;
    }

    if (!SessionSearch.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[Client] SessionSearch is invalid"));
        return;
    }

    int32 ResultCount = SessionSearch->SearchResults.Num();
    UE_LOG(LogTemp, Log, TEXT("[Client] FindSessions completed: Success=%d / Found=%d"),
           bWasSuccessful, ResultCount);

    if (ResultCount > 0)
    {
        // 🔥 이미 세션이 존재한다면 제거
        if (SessionInterface->GetNamedSession(NAME_GameSession))
        {
            UE_LOG(LogTemp, Warning, TEXT("[Client] Destroying existing session before JoinSession"));
            SessionInterface->DestroySession(NAME_GameSession);
        }

        // 결과 상세 출력
        for (int32 i = 0; i < ResultCount; ++i)
        {
            const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[i];
            FString SessionId = Result.GetSessionIdStr();
            FString HostName = Result.Session.OwningUserName;
            int32 Ping = Result.PingInMs;

            UE_LOG(LogTemp, Log, TEXT("[Client] [%d] ID:%s | Host:%s | Ping:%d"),
                   i, *SessionId, *HostName, Ping);
        }

        UE_LOG(LogTemp, Log, TEXT("[Client] Joining first session..."));
        SessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[0]);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Client] No sessions found"));
    }
}


void ATitlePlayerController::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (Result == EOnJoinSessionCompleteResult::Success && SessionInterface.IsValid())
    {
        FString ConnectString;
        if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
        {
            UE_LOG(LogTemp, Log, TEXT("[Client] Joining session: %s"), *ConnectString);
            ClientTravel(ConnectString, TRAVEL_Absolute);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[Client] Failed to resolve connect string"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Client] JoinSession failed: %d"), (int32)Result);
    }
}