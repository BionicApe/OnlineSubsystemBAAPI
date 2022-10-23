// Copyright Epic Games, Inc. All Rights Reserved.

#include "OnlineSubsystemBAAPI.h"
#include "HAL/RunnableThread.h"
#include "OnlineAsyncTaskManagerBAAPI.h"

#include "OnlineSessionInterfaceBAAPI.h"
#include "OnlineLeaderboardInterfaceBAAPI.h"
#include "OnlineIdentityBAAPI.h"
#include "OnlineAchievementsInterfaceBAAPI.h"
#include "OnlineStoreV2InterfaceBAAPI.h"
#include "OnlinePurchaseInterfaceBAAPI.h"
#include "OnlineMessageSanitizerBAAPI.h"
#include "Stats/Stats.h"
#include "Misc/ConfigCacheIni.h"

#if WITH_ENGINE
#include "VoiceInterfaceBAAPI.h"
#endif //WITH_ENGINE

FThreadSafeCounter FOnlineSubsystemBAAPI::TaskCounter;

IOnlineSessionPtr FOnlineSubsystemBAAPI::GetSessionInterface() const
{
	return SessionInterface;
}

IOnlineFriendsPtr FOnlineSubsystemBAAPI::GetFriendsInterface() const
{
	return nullptr;
}

IOnlinePartyPtr FOnlineSubsystemBAAPI::GetPartyInterface() const
{
	return nullptr;
}

IOnlineGroupsPtr FOnlineSubsystemBAAPI::GetGroupsInterface() const
{
	return nullptr;
}

IOnlineSharedCloudPtr FOnlineSubsystemBAAPI::GetSharedCloudInterface() const
{
	return nullptr;
}

IOnlineUserCloudPtr FOnlineSubsystemBAAPI::GetUserCloudInterface() const
{
	return nullptr;
}

IOnlineEntitlementsPtr FOnlineSubsystemBAAPI::GetEntitlementsInterface() const
{
	return nullptr;
};

IOnlineLeaderboardsPtr FOnlineSubsystemBAAPI::GetLeaderboardsInterface() const
{
	return LeaderboardsInterface;
}

IOnlineVoicePtr FOnlineSubsystemBAAPI::GetVoiceInterface() const
{
#if WITH_ENGINE
	if (VoiceInterface.IsValid() && !bVoiceInterfaceInitialized)
	{	
		if (!VoiceInterface->Init())
		{
			VoiceInterface = nullptr;
		}

		bVoiceInterfaceInitialized = true;
	}

	return VoiceInterface;
#else //WITH_ENGINE
	return nullptr;
#endif //WITH_ENGINE
}

IOnlineExternalUIPtr FOnlineSubsystemBAAPI::GetExternalUIInterface() const
{
	return nullptr;
}

IOnlineTimePtr FOnlineSubsystemBAAPI::GetTimeInterface() const
{
	return nullptr;
}

IOnlineIdentityPtr FOnlineSubsystemBAAPI::GetIdentityInterface() const
{
	return IdentityInterface;
}

IOnlineTitleFilePtr FOnlineSubsystemBAAPI::GetTitleFileInterface() const
{
	return nullptr;
}

IOnlineStoreV2Ptr FOnlineSubsystemBAAPI::GetStoreV2Interface() const
{
	return StoreV2Interface;
}

IOnlinePurchasePtr FOnlineSubsystemBAAPI::GetPurchaseInterface() const
{
	return PurchaseInterface;
}

IOnlineEventsPtr FOnlineSubsystemBAAPI::GetEventsInterface() const
{
	return nullptr;
}

IOnlineAchievementsPtr FOnlineSubsystemBAAPI::GetAchievementsInterface() const
{
	return AchievementsInterface;
}

IOnlineSharingPtr FOnlineSubsystemBAAPI::GetSharingInterface() const
{
	return nullptr;
}

IOnlineUserPtr FOnlineSubsystemBAAPI::GetUserInterface() const
{
	return nullptr;
}

IOnlineMessagePtr FOnlineSubsystemBAAPI::GetMessageInterface() const
{
	return nullptr;
}

IOnlinePresencePtr FOnlineSubsystemBAAPI::GetPresenceInterface() const
{
	return nullptr;
}

IOnlineChatPtr FOnlineSubsystemBAAPI::GetChatInterface() const
{
	return nullptr;
}

IOnlineStatsPtr FOnlineSubsystemBAAPI::GetStatsInterface() const
{
	return nullptr;
}

IOnlineTurnBasedPtr FOnlineSubsystemBAAPI::GetTurnBasedInterface() const
{
	return nullptr;
}

IOnlineTournamentPtr FOnlineSubsystemBAAPI::GetTournamentInterface() const
{
	return nullptr;
}

IMessageSanitizerPtr FOnlineSubsystemBAAPI::GetMessageSanitizer(int32 LocalUserNum, FString& OutAuthTypeToExclude) const
{
	return MessageSanitizerInterface;
}

bool FOnlineSubsystemBAAPI::Tick(float DeltaTime)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FOnlineSubsystemBAAPI_Tick);

	if (!FOnlineSubsystemImpl::Tick(DeltaTime))
	{
		return false;
	}

	if (OnlineAsyncTaskThreadRunnable)
	{
		OnlineAsyncTaskThreadRunnable->GameTick();
	}

	if (SessionInterface.IsValid())
	{
		SessionInterface->Tick(DeltaTime);
	}

#if WITH_ENGINE
	if (VoiceInterface.IsValid() && bVoiceInterfaceInitialized)
	{
		VoiceInterface->Tick(DeltaTime);
	}
#endif //WITH_ENGINE

	return true;
}

bool FOnlineSubsystemBAAPI::Init()
{
	const bool bBAAPIInit = true;
	
	if (bBAAPIInit)
	{
		// Create the online async task thread
		OnlineAsyncTaskThreadRunnable = new FOnlineAsyncTaskManagerBAAPI(this);
		check(OnlineAsyncTaskThreadRunnable);
		OnlineAsyncTaskThread = FRunnableThread::Create(OnlineAsyncTaskThreadRunnable, *FString::Printf(TEXT("OnlineAsyncTaskThreadBAAPI %s(%d)"), *InstanceName.ToString(), TaskCounter.Increment()), 128 * 1024, TPri_Normal);
		check(OnlineAsyncTaskThread);
		UE_LOG_ONLINE(Verbose, TEXT("Created thread (ID:%d)."), OnlineAsyncTaskThread->GetThreadID());

		SessionInterface = MakeShareable(new FOnlineSessionBAAPI(this));
		LeaderboardsInterface = MakeShareable(new FOnlineLeaderboardsBAAPI(this));
		IdentityInterface = MakeShareable(new FOnlineIdentityBAAPI(this));
		AchievementsInterface = MakeShareable(new FOnlineAchievementsBAAPI(this));
#if WITH_ENGINE
		VoiceInterface = MakeShareable(new FOnlineVoiceImpl(this));
#endif //WITH_ENGINE
		StoreV2Interface = MakeShareable(new FOnlineStoreV2BAAPI(*this));
		PurchaseInterface = MakeShareable(new FOnlinePurchaseBAAPI(*this));
		MessageSanitizerInterface = MakeShareable(new FMessageSanitizerBAAPI());
	}
	else
	{
		Shutdown();
	}

	return bBAAPIInit;
}

bool FOnlineSubsystemBAAPI::Shutdown()
{
	UE_LOG_ONLINE(VeryVerbose, TEXT("FOnlineSubsystemBAAPI::Shutdown()"));

	FOnlineSubsystemImpl::Shutdown();

	if (OnlineAsyncTaskThread)
	{
		// Destroy the online async task thread
		delete OnlineAsyncTaskThread;
		OnlineAsyncTaskThread = nullptr;
	}

	if (OnlineAsyncTaskThreadRunnable)
	{
		delete OnlineAsyncTaskThreadRunnable;
		OnlineAsyncTaskThreadRunnable = nullptr;
	}

#if WITH_ENGINE
	if (VoiceInterface.IsValid() && bVoiceInterfaceInitialized)
	{
		VoiceInterface->Shutdown();
	}
#endif //WITH_ENGINE
	
#define DESTRUCT_INTERFACE(Interface) \
	if (Interface.IsValid()) \
	{ \
		ensure(Interface.IsUnique()); \
		Interface = nullptr; \
	}
 
	// Destruct the interfaces
	DESTRUCT_INTERFACE(PurchaseInterface);
	DESTRUCT_INTERFACE(StoreV2Interface);
	DESTRUCT_INTERFACE(VoiceInterface);
	DESTRUCT_INTERFACE(AchievementsInterface);
	DESTRUCT_INTERFACE(IdentityInterface);
	DESTRUCT_INTERFACE(LeaderboardsInterface);
	DESTRUCT_INTERFACE(SessionInterface);
	DESTRUCT_INTERFACE(MessageSanitizerInterface);

#undef DESTRUCT_INTERFACE
	
	return true;
}

FString FOnlineSubsystemBAAPI::GetAppId() const
{
	return TEXT("");
}

bool FOnlineSubsystemBAAPI::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	if (FOnlineSubsystemImpl::Exec(InWorld, Cmd, Ar))
	{
		return true;
	}
	return false;
}
FText FOnlineSubsystemBAAPI::GetOnlineServiceName() const
{
	return NSLOCTEXT("OnlineSubsystemBAAPI", "OnlineServiceName", "BAAPI");
}

