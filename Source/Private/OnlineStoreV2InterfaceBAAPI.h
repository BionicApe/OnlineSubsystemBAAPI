// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineStoreInterfaceV2.h"
#include "OnlineSubsystemBAAPIPackage.h"

class FOnlineSubsystemBAAPI;
class FUniqueNetIdBAAPI;

/**
 * Implementation for online store via BAAPI interface
 */
class FOnlineStoreV2BAAPI : public IOnlineStoreV2, public TSharedFromThis<FOnlineStoreV2BAAPI, ESPMode::ThreadSafe>
{
public:
	FOnlineStoreV2BAAPI(FOnlineSubsystemBAAPI& InBAAPISubsystem);
	virtual ~FOnlineStoreV2BAAPI() = default;

public:// IOnlineStoreV2
	virtual void QueryCategories(const FUniqueNetId& UserId, const FOnQueryOnlineStoreCategoriesComplete& Delegate) override;
	virtual void GetCategories(TArray<FOnlineStoreCategory>& OutCategories) const override;
	virtual void QueryOffersByFilter(const FUniqueNetId& UserId, const FOnlineStoreFilter& Filter, const FOnQueryOnlineStoreOffersComplete& Delegate) override;
	virtual void QueryOffersById(const FUniqueNetId& UserId, const TArray<FUniqueOfferId>& OfferIds, const FOnQueryOnlineStoreOffersComplete& Delegate) override;
	virtual void GetOffers(TArray<FOnlineStoreOfferRef>& OutOffers) const override;
	virtual TSharedPtr<FOnlineStoreOffer> GetOffer(const FUniqueOfferId& OfferId) const override;

PACKAGE_SCOPE:
	void QueryOffers(const FUniqueNetIdBAAPI& UserId, const TArray<FUniqueOfferId>& OfferIds, const FOnQueryOnlineStoreOffersComplete& Delegate);

PACKAGE_SCOPE:
	FOnlineSubsystemBAAPI& BAAPISubsystem;
	TMap<FUniqueOfferId, FOnlineStoreOfferRef> AvailableOffers;

private:
	void CreateFakeOffer(const FString& Id, const FString& Title, const FString& Description, int32 Price);
};

using FOnlineStoreBAAPIPtr = TSharedPtr<FOnlineStoreV2BAAPI, ESPMode::ThreadSafe>;
using FOnlineStoreBAAPIRef = TSharedRef<FOnlineStoreV2BAAPI, ESPMode::ThreadSafe>;
