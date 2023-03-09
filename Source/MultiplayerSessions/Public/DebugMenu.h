// © 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "DebugMenu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UDebugMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void AddMultiplayerDebugMenu(
		int32 NumPlayers = 4,
		FString GameMode = FString(TEXT("FreeForAll")),
		FString LobbyMap = FString(TEXT(""))
	);

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override; /* Replaced OnLevelRemovedFromWorld() in UE 5.1 */

	// Custom delegate callbacks.

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
	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	void MenuTeardown();

	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	int32 NumPublicConnections{4};
	FString MatchType{TEXT("FreeForAll")};
	FString LobbyMapPath{TEXT("")};
};
