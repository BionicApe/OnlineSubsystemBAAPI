// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "OnlineSubsystemBAAPIModule.h"
#include "OnlineSubsystemModule.h"
#include "OnlineSubsystemNames.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemBAAPI.h"

IMPLEMENT_MODULE(FOnlineSubsystemBAAPIModule, OnlineSubsystemBAAPI);

/**
 * Class responsible for creating instance(s) of the subsystem
 */
class FOnlineFactoryBAAPI : public IOnlineFactory
{
public:

	FOnlineFactoryBAAPI() {}
	virtual ~FOnlineFactoryBAAPI() {}

	virtual IOnlineSubsystemPtr CreateSubsystem(FName InstanceName)
	{
		FOnlineSubsystemBAAPIPtr OnlineSub = MakeShared<FOnlineSubsystemBAAPI, ESPMode::ThreadSafe>(InstanceName);
		if (OnlineSub->IsEnabled())
		{
			if (!OnlineSub->Init())
			{
				UE_LOG_ONLINE(Warning, TEXT("BAAPI API failed to initialize!"));
				OnlineSub->Shutdown();
				OnlineSub = NULL;
			}
		}
		else
		{
			UE_LOG_ONLINE(Warning, TEXT("BAAPI API disabled!"));
			OnlineSub->Shutdown();
			OnlineSub = NULL;
		}

		return OnlineSub;
	}
};

void FOnlineSubsystemBAAPIModule::StartupModule()
{
	BAAPIFactory = new FOnlineFactoryBAAPI();

	// Create and register our singleton factory with the main online subsystem for easy access
	FOnlineSubsystemModule& OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemModule>("OnlineSubsystem");
	OSS.RegisterPlatformService(TEXT("BAAPI"), BAAPIFactory);
}

void FOnlineSubsystemBAAPIModule::ShutdownModule()
{
	FOnlineSubsystemModule& OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemModule>("OnlineSubsystem");
	OSS.UnregisterPlatformService(TEXT("BAAPI"));

	delete BAAPIFactory;
	BAAPIFactory = NULL;
}
