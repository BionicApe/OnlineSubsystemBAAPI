// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "IPAddress.h"

class FOnlineSubsystemBAAPI;

// from OnlineSubsystemTypes.h
TEMP_UNIQUENETIDSTRING_SUBCLASS(FUniqueNetIdBAAPI, TEXT("BAAPI"));

/** 
 * Implementation of session information
 */
class FOnlineSessionInfoBAAPI : public FOnlineSessionInfo
{
	/** Hidden on purpose */
	FOnlineSessionInfoBAAPI(const FOnlineSessionInfoBAAPI& Src) = delete;
	FOnlineSessionInfoBAAPI& operator=(const FOnlineSessionInfoBAAPI& Src) = delete;

PACKAGE_SCOPE:

	/** Constructor */
	FOnlineSessionInfoBAAPI();

	/** 
	 * Initialize a BAAPI session info with the address of this machine
	 * and an id for the session
	 */
	void Init(const FOnlineSubsystemBAAPI& Subsystem);

	/** The ip & port that the host is listening on (valid for LAN/GameServer) */
	TSharedPtr<class FInternetAddr> HostAddr;
	/** Unique Id for this session */
	FUniqueNetIdBAAPIRef SessionId;

public:

	virtual ~FOnlineSessionInfoBAAPI() {}

 	bool operator==(const FOnlineSessionInfoBAAPI& Other) const
 	{
 		return false;
 	}

	virtual const uint8* GetBytes() const override
	{
		return NULL;
	}

	virtual int32 GetSize() const override
	{
		return sizeof(uint64) + sizeof(TSharedPtr<class FInternetAddr>);
	}

	virtual bool IsValid() const override
	{
		// LAN case
		return HostAddr.IsValid() && HostAddr->IsValid();
	}

	virtual FString ToString() const override
	{
		return SessionId->ToString();
	}

	virtual FString ToDebugString() const override
	{
		return FString::Printf(TEXT("HostIP: %s SessionId: %s"), 
			HostAddr.IsValid() ? *HostAddr->ToString(true) : TEXT("INVALID"), 
			*SessionId->ToDebugString());
	}

	virtual const FUniqueNetId& GetSessionId() const override
	{
		return *SessionId;
	}
};
