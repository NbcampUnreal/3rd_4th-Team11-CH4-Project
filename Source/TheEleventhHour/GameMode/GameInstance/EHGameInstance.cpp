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

	// ✅ Dedicated 서버일 때만 세션 생성 & WaitingLevel 로 오픈
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
	Settings.bIsLANMatch = true; // 로컬 테스트용
	Settings.NumPublicConnections = 4;

	UE_LOG(LogTemp, Log, TEXT("[Server] Creating Dedicated session with map WaitingLevel..."));
	SessionInterface->CreateSession(0, NAME_GameSession, Settings);
}

void UEHGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("[Server] Dedicated session created. Opening WaitingLevel..."));
		UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Levels/Maps/WaitingLevel"), true);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Server] Failed to create session."));
	}
}

void UEHGameInstance::ConnectToIP(const FString& ServerAddress)
{
	if (SessionInterface.IsValid() && SessionInterface->GetNamedSession(NAME_GameSession))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Client] Destroying existing session before direct IP connect"));
		SessionInterface->DestroySession(NAME_GameSession);
	}

	if (APlayerController* PC = GetFirstLocalPlayerController())
	{
		FString Dest = ServerAddress;
		if (!Dest.Contains(TEXT(":")))
		{
			Dest.Append(TEXT(":7777"));
		}

		UE_LOG(LogTemp, Warning, TEXT("[Client] Direct connect to %s"), *Dest);
		PC->ClientTravel(Dest, TRAVEL_Absolute);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Client] No PlayerController to travel"));
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

			// ✅ 안전한 호출 (PlayerController가 없으면 지연 호출)
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
			{
				PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[Client] No PlayerController yet. Delaying travel."));
				FTimerHandle DelayHandle;
				GetWorld()->GetTimerManager().SetTimer(DelayHandle, FTimerDelegate::CreateLambda([=, this]()
				{
					if (APlayerController* PC2 = UGameplayStatics::GetPlayerController(GetWorld(), 0))
					{
						PC2->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
					}
				}), 0.5f, false);

			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Client] JoinSession failed: %d"), (int32)Result);
	}
}

