// © 2023 Will Roberts

#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

#include "Logger.h"

/*************
Public Methods
*************/

// UMultiplayerSessionsSubsystem constructs a new instance, binds delegates, and saves a pointer to the OnlineSubsystem's SessionInterface.
UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
    CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
    FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
    JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
    DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
    StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (!OnlineSubsystem)
    {
        Logger::Log(FString(TEXT("MultiplayerSessionsSubsystem: Failed to get OnlineSubsystem")), true);
        return;
    }

    SessionInterface = OnlineSubsystem->GetSessionInterface();
}

// CreateSession destroys any existing session before creating a new online session.
void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
    if (!SessionInterface.IsValid())
    {
        Logger::Log(FString(TEXT("CreateSession: Failed to get SessionInterface")), true);
        return;
    }

    auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (ExistingSession != nullptr)
    {
        Logger::Log(FString(TEXT("Destroying existing session...")), false);
        bCreateSessionOnDestroy = true;
        LastNumPublicConnections = NumPublicConnections;
        LastMatchType = MatchType;

        DestroySession();
    }

    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

    LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
    LastSessionSettings->bAllowJoinInProgress = true;
    LastSessionSettings->bAllowJoinViaPresence = true;
    LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
    LastSessionSettings->bShouldAdvertise = true;
    LastSessionSettings->bUseLobbiesIfAvailable = true;
    LastSessionSettings->bUsesPresence = true;
    LastSessionSettings->BuildUniqueId = 1; // For testing, to share sessions.
    LastSessionSettings->NumPublicConnections = NumPublicConnections;
    LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    bool WasSuccessful = SessionInterface->CreateSession(
        *LocalPlayer->GetPreferredUniqueNetId(),
        NAME_GameSession,
        *LastSessionSettings
    );
    if (!WasSuccessful)
    {
        Logger::Log(FString(TEXT("CreateSession: Failed to create session")), true);
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        MultiplayerOnCreateSessionComplete.Broadcast(false);
    }
}

// FindSessions searches for sessions and saves the results.
void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
    if (!SessionInterface.IsValid())
    {
        Logger::Log(FString(TEXT("FindSessions: Failed to get SessionInterface")), true);
        return;
    }

    // Configure search parameters.
    FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
    LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
    LastSessionSearch->MaxSearchResults = MaxSearchResults;
    LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
    LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    // Use first local player's unique net ID to find sessions.
    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    bool Successful = SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef());
    if (!Successful)
    {
        Logger::Log(FString(TEXT("FindSessions: Failed to find sessions")), true);
        // Clear the current delegate.
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
        // Broadcast the custom delegate.
        MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
        return;
    }
}

// JoinSession joins the specified game session with a player's unique ID.
void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult &SessionResult)
{
    if (!SessionInterface.IsValid())
    {
        Logger::Log(FString(TEXT("JoinSession: Failed to get SessionInterface")), true);
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
        return;
    }
    JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
    
    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    bool Successful = SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult);
    if (!Successful)
    {
        Logger::Log(FString(TEXT("JoinSession: Failed to join session")), true);
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
    }
}

// DestroySession destroys the current session.
void UMultiplayerSessionsSubsystem::DestroySession()
{
    if (!SessionInterface.IsValid())
    {
        Logger::Log(FString(TEXT("DestroySession: Failed to get SessionInterface")), true);
        MultiplayerOnDestroySessionComplete.Broadcast(false);
        return;
    }
    DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

    bool Successful = SessionInterface->DestroySession(NAME_GameSession);
    if (!Successful)
    {
        Logger::Log(FString(TEXT("DestroySession: Failed to destroy session")), true);
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
        MultiplayerOnDestroySessionComplete.Broadcast(false);
    }
}

// StartSession marks the online session as in-progress.
void UMultiplayerSessionsSubsystem::StartSession()
{
    if (!SessionInterface.IsValid())
    {
        Logger::Log(FString(TEXT("StartSession: Failed to get SessionInterface")), true);
        MultiplayerOnStartSessionComplete.Broadcast(false);
        return;
    }
    StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);

    bool Successful = SessionInterface->StartSession(NAME_GameSession);
    if (!Successful)
    {
        Logger::Log(FString(TEXT("StartSession: Failed to start session")), true);
        SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
        MultiplayerOnStartSessionComplete.Broadcast(false);
    }
}

/****************
Protected Methods
****************/

// OnCreateSessionComplete clears its delegate handle and broadcasts its result.
void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (!SessionInterface)
    {
        Logger::Log(FString(TEXT("OnCreateSessionComplete: Failed to get SessionInterface")), true);
        return;
    }

    Logger::Log(FString(TEXT("Created session")), false);
    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
    MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

// OnFindSessionsComplete clears its delegate handle and broadcasts its result.
void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (!SessionInterface)
    {
        Logger::Log(FString(TEXT("OnFindSessionsComplete: Failed to get SessionInterface")), true);
        return;
    }
    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

    if (LastSessionSearch->SearchResults.Num() <= 0)
    {
        Logger::Log(FString(TEXT("OnFindSessionsComplete: No results found")), true);
        MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
        return;
    }

    MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

// OnJoinSessionComplete clears its delegate handle and broadcasts its result.
void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (!SessionInterface)
    {
        Logger::Log(FString(TEXT("OnJoinSessionComplete: Failed to get SessionInterface")), true);
        return;
    }
    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

    Logger::Log(FString(TEXT("Joined session")), false);
    MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

// OnDestroySessionComplete clears its delegate handle and broadcasts its result.
// If `bCreateSessionOnDestroy` is true, this also creates a new online session.
void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (!SessionInterface)
    {
        Logger::Log(FString(TEXT("OnDestroySessionComplete: Failed to get SessionInterface")), true);
        return;
    }
    SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);

    if (bWasSuccessful && bCreateSessionOnDestroy)
    {
        bCreateSessionOnDestroy = false;
        CreateSession(LastNumPublicConnections, LastMatchType);
    }

    Logger::Log(FString(TEXT("Destroyed session")), false);
    MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

// OnStartSessionComplete clears its delegate handle and broadcasts its result.
void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (!SessionInterface)
    {
        Logger::Log(FString(TEXT("OnStartSessionComplete: Failed to get SessionInterface")), true);
        return;
    }
    SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);

    if (!bWasSuccessful)
    {
        Logger::Log(FString(TEXT("OnStartSessionComplete: Failed to start session")), true);
    }

    Logger::Log(FString(TEXT("Started session")), false);
    MultiplayerOnStartSessionComplete.Broadcast(bWasSuccessful);
}