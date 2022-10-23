// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "VoiceInterfaceImpl.h"
#include "Online/CoreOnline.h"
#include "OnlineSubsystemTypes.h"
#include "Interfaces/VoiceInterface.h"
#include "Net/VoiceDataCommon.h"
#include "OnlineSubsystemUtilsPackage.h"

class ONLINESUBSYSTEMBAAPI_API FOnlineVoiceImplBAAPI : public FOnlineVoiceImpl {
PACKAGE_SCOPE:
	FOnlineVoiceImplBAAPI() : FOnlineVoiceImpl()
	{};

public:
	/** Constructor */
	FOnlineVoiceImplBAAPI(class IOnlineSubsystem* InOnlineSubsystem) :
		FOnlineVoiceImpl(InOnlineSubsystem)
	{
	};

	virtual ~FOnlineVoiceImplBAAPI() override {}
};

typedef TSharedPtr<FOnlineVoiceImpl, ESPMode::ThreadSafe> FOnlineVoiceImplPtr;
