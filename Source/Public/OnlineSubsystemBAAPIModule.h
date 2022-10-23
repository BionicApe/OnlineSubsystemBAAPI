// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
 * Online subsystem module class  (BAAPI Implementation)
 * Code related to the loading of the BAAPI module
 */
class FOnlineSubsystemBAAPIModule : public IModuleInterface
{
private:

	/** Class responsible for creating instance(s) of the subsystem */
	class FOnlineFactoryBAAPI* BAAPIFactory;

public:

	FOnlineSubsystemBAAPIModule() : 
		BAAPIFactory(NULL)
	{}

	virtual ~FOnlineSubsystemBAAPIModule() {}

	// IModuleInterface

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool SupportsDynamicReloading() override
	{
		return false;
	}

	virtual bool SupportsAutomaticShutdown() override
	{
		return false;
	}
};
