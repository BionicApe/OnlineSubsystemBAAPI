// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineAsyncTaskManager.h"

/**
 *	BAAPI version of the async task manager to register the various BAAPI callbacks with the engine
 */
class FOnlineAsyncTaskManagerBAAPI : public FOnlineAsyncTaskManager
{
protected:

	/** Cached reference to the main online subsystem */
	class FOnlineSubsystemBAAPI* BAAPISubsystem;

public:

	FOnlineAsyncTaskManagerBAAPI(class FOnlineSubsystemBAAPI* InOnlineSubsystem)
		: BAAPISubsystem(InOnlineSubsystem)
	{
	}

	~FOnlineAsyncTaskManagerBAAPI() 
	{
	}

	// FOnlineAsyncTaskManager
	virtual void OnlineTick() override;
};
