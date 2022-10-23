// Copyright Epic Games, Inc. All Rights Reserved.

#include "OnlineAsyncTaskManagerBAAPI.h"

void FOnlineAsyncTaskManagerBAAPI::OnlineTick()
{
	check(BAAPISubsystem);
	check(FPlatformTLS::GetCurrentThreadId() == OnlineThreadId || !FPlatformProcess::SupportsMultithreading());
}

