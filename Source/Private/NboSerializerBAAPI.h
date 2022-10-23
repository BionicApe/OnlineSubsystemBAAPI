// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemBAAPITypes.h"
#include "NboSerializer.h"

/**
 * Serializes data in network byte order form into a buffer
 */
class FNboSerializeToBufferBAAPI : public FNboSerializeToBuffer
{
public:
	/** Default constructor zeros num bytes*/
	FNboSerializeToBufferBAAPI() :
		FNboSerializeToBuffer(512)
	{
	}

	/** Constructor specifying the size to use */
	FNboSerializeToBufferBAAPI(uint32 Size) :
		FNboSerializeToBuffer(Size)
	{
	}

	/**
	 * Adds BAAPI session info to the buffer
	 */
 	friend inline FNboSerializeToBufferBAAPI& operator<<(FNboSerializeToBufferBAAPI& Ar, const FOnlineSessionInfoBAAPI& SessionInfo)
 	{
		check(SessionInfo.HostAddr.IsValid());
		// Skip SessionType (assigned at creation)
		Ar << *SessionInfo.SessionId;
		Ar << *SessionInfo.HostAddr;
		return Ar;
 	}

	/**
	 * Adds BAAPI Unique Id to the buffer
	 */
	friend inline FNboSerializeToBufferBAAPI& operator<<(FNboSerializeToBufferBAAPI& Ar, const FUniqueNetIdBAAPI& UniqueId)
	{
		Ar << UniqueId.UniqueNetIdStr;
		return Ar;
	}
};

/**
 * Class used to write data into packets for sending via system link
 */
class FNboSerializeFromBufferBAAPI : public FNboSerializeFromBuffer
{
public:
	/**
	 * Initializes the buffer, size, and zeros the read offset
	 */
	FNboSerializeFromBufferBAAPI(uint8* Packet,int32 Length) :
		FNboSerializeFromBuffer(Packet,Length)
	{
	}

	/**
	 * Reads BAAPI session info from the buffer
	 */
 	friend inline FNboSerializeFromBufferBAAPI& operator>>(FNboSerializeFromBufferBAAPI& Ar, FOnlineSessionInfoBAAPI& SessionInfo)
 	{
		check(SessionInfo.HostAddr.IsValid());
		// Skip SessionType (assigned at creation)
		SessionInfo.SessionId = FUniqueNetIdBAAPI::Create();
		Ar >> const_cast<FUniqueNetIdBAAPI&>(*SessionInfo.SessionId);
		Ar >> *SessionInfo.HostAddr;
		return Ar;
 	}

	/**
	 * Reads BAAPI Unique Id from the buffer
	 */
	friend inline FNboSerializeFromBufferBAAPI& operator>>(FNboSerializeFromBufferBAAPI& Ar, FUniqueNetIdBAAPI& UniqueId)
	{
		Ar >> UniqueId.UniqueNetIdStr;
		return Ar;
	}
};
