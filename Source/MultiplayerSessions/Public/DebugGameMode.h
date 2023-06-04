// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DebugGameMode.generated.h"

/*
 * ADebugGameMode provides a Game Mode class which overrides login and logout handlers.
 * The new handlers will print players' names as they connect or disconnect, as well as total player counts.
 */
UCLASS()
class MULTIPLAYERSESSIONS_API ADebugGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* ExitingPlayer) override;
};