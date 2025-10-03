#include "GameMode/GameInstance/EHGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

void UEHGameInstance::Init()
{
    Super::Init();

    if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
    {
        SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UEHGameInstance::OnCreateSessionComplete);
            SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UEHGameInstance::OnFindSessionsComplete);
            SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UEHGameInstance::OnJoinSessionComplete);
        }
    }

    if (IsRunningDedicatedServer() && !bSessionCreated)
    {
        bSessionCreated = true;
        UE_LOG(LogTemp, Warning, TEXT("[Server] Creating dedicated session"));
        HostDedicatedSession();
    }
}

void UEHGameInstance::HostDedicatedSession()
{
    if (!SessionInterface.IsValid()) return;

    FOnlineSessionSettings Settings;
    Settings.bIsDedicated = true;
    Settings.bShouldAdvertise = true;
    Settings.bIsLANMatch = true;
    Settings.NumPublicConnections = 4;

    // 원하는 맵 주소를 직접 등록
    Settings.Set(FName("MAPNAME"), FString("/Game/Levels/Maps/WaitingLevel"),
                 EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    UE_LOG(LogTemp, Log, TEXT("[Server] Creating Dedicated session with map WaitingLevel..."));
    SessionInterface->CreateSession(0, NAME_GameSession, Settings);
}

void UEHGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("[Server] Dedicated session created."));
        // ★ Dedicated 서버는 그냥 WaitingLevel 유지 (ServerTravel 불필요)
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Server] Failed to create session."));
    }
}

void UEHGameInstance::FindSessionsAndJoin()
{
    UE_LOG(LogTemp, Warning, TEXT("[Client] >>> FindSessionsAndJoin called"));

    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[Client] SessionInterface invalid"));
        return;
    }

    const ULocalPlayer* LP = GetFirstGamePlayer();
    if (!LP)
    {
        UE_LOG(LogTemp, Error, TEXT("[Client] No LocalPlayer"));
        return;
    }

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->bIsLanQuery = true;
    SessionSearch->MaxSearchResults = 20;

    UE_LOG(LogTemp, Warning, TEXT("[Client] Start FindSessions"));
    bool bStarted = SessionInterface->FindSessions(*LP->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
    UE_LOG(LogTemp, Warning, TEXT("[Client] FindSessions returned %d"), bStarted ? 1 : 0);
}

void UEHGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (!bWasSuccessful || !SessionSearch.IsValid()) return;

    if (SessionSearch->SearchResults.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("[Client] Joining first found session..."));

        // ✅ 기존 세션 제거
        if (SessionInterface->GetNamedSession(NAME_GameSession))
        {
            UE_LOG(LogTemp, Warning, TEXT("[Client] Destroying existing session before JoinSession"));
            SessionInterface->DestroySession(NAME_GameSession);
        }

        SessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[0]);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Client] No sessions found."));
    }
}


void UEHGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        FString ConnectString;
        if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
        {
            UE_LOG(LogTemp, Log, TEXT("[Client] Travel to %s"), *ConnectString);
            GetFirstLocalPlayerController()->ClientTravel(ConnectString, TRAVEL_Absolute);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[Client] Failed to resolve connect string."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Client] JoinSession failed: %d"), (int32)Result);
    }
}