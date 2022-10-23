// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePurchaseInterface.h"
#include "OnlineSubsystemBAAPITypes.h"

class FOnlineSubsystemBAAPI;

class FOnlinePurchaseBAAPI
	: public IOnlinePurchase
	, public TSharedFromThis<FOnlinePurchaseBAAPI, ESPMode::ThreadSafe>
{
public:
	FOnlinePurchaseBAAPI(FOnlineSubsystemBAAPI& InBAAPISubsystem);
	virtual ~FOnlinePurchaseBAAPI();

	void Tick();

public:
	//~ Begin IOnlinePurchase Interface
	virtual bool IsAllowedToPurchase(const FUniqueNetId& UserId) override;
	virtual void Checkout(const FUniqueNetId& UserId, const FPurchaseCheckoutRequest& CheckoutRequest, const FOnPurchaseCheckoutComplete& Delegate) override;
	virtual void FinalizePurchase(const FUniqueNetId& UserId, const FString& ReceiptId) override;
	virtual void RedeemCode(const FUniqueNetId& UserId, const FRedeemCodeRequest& RedeemCodeRequest, const FOnPurchaseRedeemCodeComplete& Delegate) override;
	virtual void QueryReceipts(const FUniqueNetId& UserId, bool bRestoreReceipts, const FOnQueryReceiptsComplete& Delegate) override;
	virtual void GetReceipts(const FUniqueNetId& UserId, TArray<FPurchaseReceipt>& OutReceipts) const override;
	virtual void FinalizeReceiptValidationInfo(const FUniqueNetId& UserId, FString& InReceiptValidationInfo, const FOnFinalizeReceiptValidationInfoComplete& Delegate) override;
	//~ End IOnlinePurchase Interface

PACKAGE_SCOPE:
	void CheckoutSuccessfully(const FUniqueNetIdBAAPI& UserId, TSharedPtr<FOnlineStoreOffer> Offer);

PACKAGE_SCOPE:
	/** Pointer back to our parent subsystem */
	FOnlineSubsystemBAAPI& BAAPISubsystem;

	/** Cached receipts information per user */
	TMap<FUniqueNetIdBAAPI, TArray<FPurchaseReceipt> > UserFakeReceipts;

	/** Do we have a purchase currently in progress? */
	TOptional<FOnPurchaseCheckoutComplete> PendingPurchaseDelegate;

	TOptional<double> PendingPurchaseFailTime;
};

using FOnlinePurchaseBAAPIPtr = TSharedPtr<FOnlinePurchaseBAAPI, ESPMode::ThreadSafe>;
using FOnlinePurchaseBAAPIRef = TSharedRef<FOnlinePurchaseBAAPI, ESPMode::ThreadSafe>;
