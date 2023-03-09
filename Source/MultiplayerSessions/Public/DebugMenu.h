// © 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "DebugMenu.generated.h"

/*
 * UDebugMenu provides a widget with buttons for hosting and joining games.
 * The menu binds a series of custom delegate callbacks used for session management.
 * When creating the widget, callers can provide the number of players, game mode, and lobby map for the session.
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UDebugMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void AddMultiplayerDebugMenu(
		int32 MaxSearchResults = 1000,
		int32 NumPlayers = 8,
		FString GameMode = FString(TEXT("YourGameMode")),
		FString LobbyMap = FString(TEXT("YourLobbyMap"))
	);

protected:
	virtual bool Initialize() override;

	// Replaced OnLevelRemovedFromWorld() in UE 5.1.
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

	// Not dynamic, and therefore not a UFUNCTION.
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);

	// Not dynamic, and therefore not a UFUNCTION.
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:
	void Destroy();

	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	/********************
	Host and Join buttons
	********************/

	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	/************************
	Saved Session Information
	************************/

	int32 SessionSearchLimit{0};
	int32 NumPublicConnections{0};
	FString MatchType{TEXT("")};
	FString LobbyMapPath{TEXT("")};
};
