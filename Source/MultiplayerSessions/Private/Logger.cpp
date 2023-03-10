// © 2023 Will Roberts

#include "Logger.h"

#include "Engine/Engine.h"
#include "Logging/LogCategory.h"
#include "Logging/LogVerbosity.h"

DEFINE_LOG_CATEGORY(LogMultiplayerSessions);

/*************
Public Methods
*************/

Logger::Logger()
{
}

Logger::~Logger()
{
}

// Log writes a log message to the screen via GEngine and to file log via ULOG.
void Logger::Log(FString Message, bool bIsError)
{
    // Log the message to the screen if the engine is available.
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1, // Message index -1 avoids overwriting existing log messages.
            15.f, // Show the message for 15 seconds.
            bIsError ? FColor::Red : FColor::Cyan,
            Message
        );
    }

    // Log the message to file.
    ELogVerbosity::Type LogVerbosity = bIsError ? ELogVerbosity::Error : ELogVerbosity::Display;
    if (bIsError)
    {
        UE_LOG(LogMultiplayerSessions, Error, TEXT("%s"), *Message);
    }
    else
    {
        UE_LOG(LogMultiplayerSessions, Display, TEXT("%s"), *Message);
    }
}