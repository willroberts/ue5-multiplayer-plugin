// © 2023 Will Roberts

#include "Components/Button.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

#include "DebugMenu.h"
#include "Logger.h"

/*************
Public Methods
*************/

// AddMultiplayerDebugMenu implements the user-facing setup method for new debug menus.
// This function is callable from Blueprints.
void UDebugMenu::AddMultiplayerDebugMenu(int32 MaxSearchResults, int32 NumPlayers, FString GameMode, FString LobbyMap)
{
    // Save arguments for later reference.
    SessionSearchLimit = MaxSearchResults;
    NumPublicConnections = NumPlayers;
    MatchType = GameMode;
    LobbyMapPath = FString::Printf(TEXT("%s?listen"), *LobbyMap);

    // Add the widget to the viewport.
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;

    // Get the player controller from the World.
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        return;
    }

    // Configure mouse input for the menu widget.
    FInputModeUIOnly InputModeData;
    InputModeData.SetWidgetToFocus(TakeWidget());
    InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PlayerController->SetInputMode(InputModeData);
    PlayerController->SetShowMouseCursor(true);

    // Get the Subsystem from the GameInstance.
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance)
    {
        return;
    }
    MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
    if (!MultiplayerSessionsSubsystem)
    {
        return;
    }

    // Bind callbacks.
    MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
    MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
    MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
    MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
    MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
}

/****************
Protected Methods
****************/

// Initialize adds click handlers to the Host and Join buttons.
bool UDebugMenu::Initialize()
{
    if (!Super::Initialize())
    {
        return false;
    }

    if (HostButton)
    {
        HostButton->OnClicked.AddDynamic(this, &UDebugMenu::HostButtonClicked);
    }

    if (JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &UDebugMenu::JoinButtonClicked);
    }

    return true;
}

// NativeDestruct calls the custom UDebugMenu::Destroy() function when the parent widget is destroyed.
void UDebugMenu::NativeDestruct()
{
    Destroy();
    Super::NativeDestruct();
}

// OnCreateSession is the delegate callback for session creation.
// When session creation was successful, initiates server travel to the lobby map.
void UDebugMenu::OnCreateSession(bool bWasSuccessful)
{
    if (!bWasSuccessful)
    {
        Logger::Log(FString(TEXT("Failed to create session")), true);
        HostButton->SetIsEnabled(true);
        return;
    }
    Logger::Log(FString(TEXT("Created session successfully")), false);

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    World->ServerTravel(LobbyMapPath);
}

// OnFindSessions is the delegate callback for session search.
// When a valid session is found, initiate a session join.
void UDebugMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
    if (MultiplayerSessionsSubsystem == nullptr)
    {
        return;
    }

    if (!bWasSuccessful || SessionResults.Num() == 0)
    {
        JoinButton->SetIsEnabled(true);
        return;
    }

    for (auto Result : SessionResults)
    {
        FString SettingsValue;
        Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
        if (SettingsValue != MatchType)
        {
            continue;
        }

        // Join the session.
        MultiplayerSessionsSubsystem->JoinSession(Result);
        return;
    }
}

// OnJoinSession is the delegate callback for session joins.
// When joining succeeds, initiate client travel to the session's platform-specific connection address.
void UDebugMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        JoinButton->SetIsEnabled(true);
        return;
    }

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem)
    {
        return;
    }

    IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        return;
    }
    FString Address;
    SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

    APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
    if (!PlayerController)
    {
        return;
    }
    PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

// OnDestroySession is the delegate callback for session destruction.
// Not yet implemented.
void UDebugMenu::OnDestroySession(bool bWasSuccessful)
{
    return;
}

// OnStartSession is the delegate callback for session initiation.
// Not yet implemented.
void UDebugMenu::OnStartSession(bool bWasSuccessful)
{
    if (!bWasSuccessful)
    {
        Logger::Log(FString(TEXT("Failed to start session")), true);
        return;
    }
    Logger::Log(FString(TEXT("Started session successfully")), false);
}

/**************
Private Methods
**************/

// HostButtonClicked temporarily disables the Host button before initiating session creation.
void UDebugMenu::HostButtonClicked()
{
    HostButton->SetIsEnabled(false);

    if (!MultiplayerSessionsSubsystem)
    {
        return;
    }
    MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType); 
}

// JoinButtonClicked temporarily disables the Join button before initiating session search.
void UDebugMenu::JoinButtonClicked()
{
    JoinButton->SetIsEnabled(false);

    if (!MultiplayerSessionsSubsystem)
    {
        return;
    }
    MultiplayerSessionsSubsystem->FindSessions(10000);
}

// Destroy the menu widget and return control to the player controller.
void UDebugMenu::Destroy()
{
    // Remove the Widget from the UI.
    RemoveFromParent();

    // Get the PlayerController from the World.
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        return;
    }

    // Return input control to the player.
    FInputModeGameOnly InputModeData;
    PlayerController->SetInputMode(InputModeData);
    PlayerController->SetShowMouseCursor(false);
}
