// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMultiplayerSessions, Log, All);

/*
 * Logger provides static methods for logging information.
 */
class MULTIPLAYERSESSIONS_API Logger
{
public:
	Logger();
	~Logger();

	static void Log(FString Message, bool bIsError);
};