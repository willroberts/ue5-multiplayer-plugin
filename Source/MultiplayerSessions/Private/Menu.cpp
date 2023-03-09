// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

void UMenu::MenuSetup(int32 NumPlayers, FString GameMode, FString LobbyMap)
{
    NumPublicConnections = NumPlayers;
    MatchType = GameMode;
    LobbyMapPath = FString::Printf(TEXT("%s?listen"), *LobbyMap);

    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;

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

    FInputModeUIOnly InputModeData;
    InputModeData.SetWidgetToFocus(TakeWidget());
    InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PlayerController->SetInputMode(InputModeData);
    PlayerController->SetShowMouseCursor(true);

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

bool UMenu::Initialize()
{
    if (!Super::Initialize())
    {
        return false;
    }

    if (HostButton)
    {
        HostButton->OnClicked.AddDynamic(this, &UMenu::HostButtonClicked);
    }

    if (JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &UMenu::JoinButtonClicked);
    }

    return true;
}

void UMenu::NativeDestruct()
{
    MenuTeardown();
    Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
    if (!bWasSuccessful)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                15.f,
                FColor::Red,
                FString(TEXT("Failed to create session"))
            );
        }
        HostButton->SetIsEnabled(true);
        return;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            15.f,
            FColor::Purple,
            FString(TEXT("Created session successfully"))
        );
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    World->ServerTravel(LobbyMapPath);
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
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

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
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

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
    if (!bWasSuccessful)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                15.f,
                FColor::Red,
                FString(TEXT("Failed to start session"))
            );
        }
        return;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            15.f,
            FColor::Purple,
            FString(TEXT("Started session successfully"))
        );
    }

    /*
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Hardcoded!
    World->ServerTravel("/Game/MultiplayerTesting/Maps/Lobby?listen");
    */
}

void UMenu::HostButtonClicked()
{
    HostButton->SetIsEnabled(false);

    if (!MultiplayerSessionsSubsystem)
    {
        return;
    }
    MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType); 
}

void UMenu::JoinButtonClicked()
{
    JoinButton->SetIsEnabled(false);

    if (!MultiplayerSessionsSubsystem)
    {
        return;
    }
    MultiplayerSessionsSubsystem->FindSessions(10000);
}

// Destroy the menu widget and return control to the player controller.
void UMenu::MenuTeardown()
{
    RemoveFromParent();
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

    FInputModeGameOnly InputModeData;
    PlayerController->SetInputMode(InputModeData);
    PlayerController->SetShowMouseCursor(false);
}
