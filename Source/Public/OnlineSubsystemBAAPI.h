// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemImpl.h"
#include "OnlineSubsystemBAAPIPackage.h"
#include "HAL/ThreadSafeCounter.h"

class FOnlineAchievementsBAAPI;
class FOnlineIdentityBAAPI;
class FOnlineLeaderboardsBAAPI;
class FOnlineSessionBAAPI;
class FOnlineVoiceImpl;

/** Forward declarations of all interface classes */
typedef TSharedPtr<class FOnlineSessionBAAPI, ESPMode::ThreadSafe> FOnlineSessionBAAPIPtr;
typedef TSharedPtr<class FOnlineProfileBAAPI, ESPMode::ThreadSafe> FOnlineProfileBAAPIPtr;
typedef TSharedPtr<class FOnlineFriendsBAAPI, ESPMode::ThreadSafe> FOnlineFriendsBAAPIPtr;
typedef TSharedPtr<class FOnlineUserCloudBAAPI, ESPMode::ThreadSafe> FOnlineUserCloudBAAPIPtr;
typedef TSharedPtr<class FOnlineLeaderboardsBAAPI, ESPMode::ThreadSafe> FOnlineLeaderboardsBAAPIPtr;
typedef TSharedPtr<class FOnlineExternalUIBAAPI, ESPMode::ThreadSafe> FOnlineExternalUIBAAPIPtr;
typedef TSharedPtr<class FOnlineIdentityBAAPI, ESPMode::ThreadSafe> FOnlineIdentityBAAPIPtr;
typedef TSharedPtr<class FOnlineAchievementsBAAPI, ESPMode::ThreadSafe> FOnlineAchievementsBAAPIPtr;
typedef TSharedPtr<class FOnlineStoreV2BAAPI, ESPMode::ThreadSafe> FOnlineStoreV2BAAPIPtr;
typedef TSharedPtr<class FOnlinePurchaseBAAPI, ESPMode::ThreadSafe> FOnlinePurchaseBAAPIPtr;
typedef TSharedPtr<class FMessageSanitizerBAAPI, ESPMode::ThreadSafe> FMessageSanitizerBAAPIPtr;
#if WITH_ENGINE
typedef TSharedPtr<class FOnlineVoiceImpl, ESPMode::ThreadSafe> FOnlineVoiceImplPtr;
#endif //WITH_ENGINE

/**
 *	OnlineSubsystemBAAPI - Implementation of the online subsystem for BAAPI services
 */
class ONLINESUBSYSTEMBAAPI_API FOnlineSubsystemBAAPI : 
	public FOnlineSubsystemImpl
{

public:

	virtual ~FOnlineSubsystemBAAPI() = default;

	// IOnlineSubsystem

	virtual IOnlineSessionPtr GetSessionInterface() const override;
	virtual IOnlineFriendsPtr GetFriendsInterface() const override;
	virtual IOnlinePartyPtr GetPartyInterface() const override;
	virtual IOnlineGroupsPtr GetGroupsInterface() const override;
	virtual IOnlineSharedCloudPtr GetSharedCloudInterface() const override;
	virtual IOnlineUserCloudPtr GetUserCloudInterface() const override;
	virtual IOnlineEntitlementsPtr GetEntitlementsInterface() const override;
	virtual IOnlineLeaderboardsPtr GetLeaderboardsInterface() const override;
	virtual IOnlineVoicePtr GetVoiceInterface() const override;
	virtual IOnlineExternalUIPtr GetExternalUIInterface() const override;	
	virtual IOnlineTimePtr GetTimeInterface() const override;
	virtual IOnlineIdentityPtr GetIdentityInterface() const override;
	virtual IOnlineTitleFilePtr GetTitleFileInterface() const override;
	virtual IOnlineStoreV2Ptr GetStoreV2Interface() const override;
	virtual IOnlinePurchasePtr GetPurchaseInterface() const override;
	virtual IOnlineEventsPtr GetEventsInterface() const override;
	virtual IOnlineAchievementsPtr GetAchievementsInterface() const override;
	virtual IOnlineSharingPtr GetSharingInterface() const override;
	virtual IOnlineUserPtr GetUserInterface() const override;
	virtual IOnlineMessagePtr GetMessageInterface() const override;
	virtual IOnlinePresencePtr GetPresenceInterface() const override;
	virtual IOnlineChatPtr GetChatInterface() const override;
	virtual IOnlineStatsPtr GetStatsInterface() const override;
	virtual IOnlineTurnBasedPtr GetTurnBasedInterface() const override;
	virtual IOnlineTournamentPtr GetTournamentInterface() const override;
	virtual IMessageSanitizerPtr GetMessageSanitizer(int32 LocalUserNum, FString& OutAuthTypeToExclude) const override;

	virtual bool Init() override;
	virtual bool Shutdown() override;
	virtual FString GetAppId() const override;
	virtual bool Exec(class UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	virtual FText GetOnlineServiceName() const override;

	// FTSTickerObjectBase
	
	virtual bool Tick(float DeltaTime) override;

	// FOnlineSubsystemBAAPI

PACKAGE_SCOPE:

	/** Only the factory makes instances */
	FOnlineSubsystemBAAPI() = delete;
	explicit FOnlineSubsystemBAAPI(FName InInstanceName) :
		FOnlineSubsystemImpl(TEXT("BAAPI"), InInstanceName),
		SessionInterface(nullptr),
		VoiceInterface(nullptr),
		bVoiceInterfaceInitialized(false),
		LeaderboardsInterface(nullptr),
		IdentityInterface(nullptr),
		AchievementsInterface(nullptr),
		StoreV2Interface(nullptr),
		MessageSanitizerInterface(nullptr),
		OnlineAsyncTaskThreadRunnable(nullptr),
		OnlineAsyncTaskThread(nullptr)
	{}

private:

	/** Interface to the session services */
	FOnlineSessionBAAPIPtr SessionInterface;

	/** Interface for voice communication */
	mutable IOnlineVoicePtr VoiceInterface;

	/** Interface for voice communication */
	mutable bool bVoiceInterfaceInitialized;

	/** Interface to the leaderboard services */
	FOnlineLeaderboardsBAAPIPtr LeaderboardsInterface;

	/** Interface to the identity registration/auth services */
	FOnlineIdentityBAAPIPtr IdentityInterface;

	/** Interface for achievements */
	FOnlineAchievementsBAAPIPtr AchievementsInterface;

	/** Interface for store */
	FOnlineStoreV2BAAPIPtr StoreV2Interface;

	/** Interface for purchases */
	FOnlinePurchaseBAAPIPtr PurchaseInterface;

	/** Interface for message sanitizing */
	FMessageSanitizerBAAPIPtr MessageSanitizerInterface;

	/** Online async task runnable */
	class FOnlineAsyncTaskManagerBAAPI* OnlineAsyncTaskThreadRunnable;

	/** Online async task thread */
	class FRunnableThread* OnlineAsyncTaskThread;

	// task counter, used to generate unique thread names for each task
	static FThreadSafeCounter TaskCounter;
};

typedef TSharedPtr<FOnlineSubsystemBAAPI, ESPMode::ThreadSafe> FOnlineSubsystemBAAPIPtr;

