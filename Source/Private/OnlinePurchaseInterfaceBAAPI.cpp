// Copyright Epic Games, Inc. All Rights Reserved.

#include "OnlinePurchaseInterfaceBAAPI.h"
#include "OnlineStoreV2InterfaceBAAPI.h"
#include "OnlineSubsystemBAAPI.h"
#include "OnlineError.h"
#include "Misc/ConfigCacheIni.h"

namespace
{
	static FPurchaseReceipt::FReceiptOfferEntry MakeReceiptOfferEntry(const FUniqueNetIdBAAPI& BAAPIUserId, const FString& Id, const FString& Name)
	{
		FPurchaseReceipt::FReceiptOfferEntry OfferEntry(FString(), Id, 1);
		{
			FPurchaseReceipt::FLineItemInfo LineItem;
			LineItem.ItemName = Name;
			LineItem.UniqueId = Id;
			OfferEntry.LineItems.Emplace(MoveTemp(LineItem));
		}

		return OfferEntry;
	}
}

FOnlinePurchaseBAAPI::FOnlinePurchaseBAAPI(FOnlineSubsystemBAAPI& InBAAPISubsystem)
	: BAAPISubsystem(InBAAPISubsystem)
{
}

FOnlinePurchaseBAAPI::~FOnlinePurchaseBAAPI()
{
}

void FOnlinePurchaseBAAPI::Tick()
{
	if (PendingPurchaseFailTime.IsSet() && PendingPurchaseDelegate.IsSet())
	{
		if (FPlatformTime::Seconds() > PendingPurchaseFailTime.GetValue())
		{
			FOnPurchaseCheckoutComplete Delegate = MoveTemp(PendingPurchaseDelegate.GetValue());
			PendingPurchaseDelegate.Reset();
			PendingPurchaseFailTime.Reset();

			Delegate.ExecuteIfBound(FOnlineError(TEXT("Checkout was cancelled or timed out")), MakeShared<FPurchaseReceipt>());
		}
	}
}

bool FOnlinePurchaseBAAPI::IsAllowedToPurchase(const FUniqueNetId& UserId)
{
	return true;
}

void FOnlinePurchaseBAAPI::Checkout(const FUniqueNetId& UserId, const FPurchaseCheckoutRequest& CheckoutRequest, const FOnPurchaseCheckoutComplete& Delegate)
{
	// Lambda to wrap calling our delegate with an error and logging the message
	auto CallDelegateError = [this, &Delegate](FString&& ErrorMessage)
	{
		BAAPISubsystem.ExecuteNextTick([Delegate, MovedErrorMessage = MoveTemp(ErrorMessage)]() mutable
		{
			UE_LOG_ONLINE(Error, TEXT("%s"), *MovedErrorMessage);

			const TSharedRef<FPurchaseReceipt> PurchaseReceipt = MakeShared<FPurchaseReceipt>();
			PurchaseReceipt->TransactionState = EPurchaseTransactionState::Failed;

			Delegate.ExecuteIfBound(FOnlineError(MoveTemp(MovedErrorMessage)), PurchaseReceipt);
		});
	};

	if (CheckoutRequest.PurchaseOffers.Num() == 0)
	{
		CallDelegateError(TEXT("FOnlinePurchaseBAAPI::Checkout failed, there were no entries passed to purchase"));
		return;
	}
	else if (CheckoutRequest.PurchaseOffers.Num() != 1)
	{
		CallDelegateError(TEXT("FOnlinePurchaseBAAPI::Checkout failed, there were more than one entry passed to purchase. We currently only support one."));
		return;
	}

	check(CheckoutRequest.PurchaseOffers.IsValidIndex(0));
	const FPurchaseCheckoutRequest::FPurchaseOfferEntry& Entry = CheckoutRequest.PurchaseOffers[0];

	if (Entry.Quantity != 1)
	{
		CallDelegateError(TEXT("FOnlinePurchaseBAAPI::Checkout failed, purchase quantity not set to one. We currently only support one."));
		return;
	}

	if (Entry.OfferId.IsEmpty())
	{
		CallDelegateError(TEXT("FOnlinePurchaseBAAPI::Checkout failed, OfferId is blank."));
		return;
	}

	const IOnlineStoreV2Ptr BAAPIStoreInt = BAAPISubsystem.GetStoreV2Interface();

	TSharedPtr<FOnlineStoreOffer> BAAPIOffer = BAAPIStoreInt->GetOffer(Entry.OfferId);
	if (!BAAPIOffer.IsValid())
	{
		CallDelegateError(TEXT("FOnlinePurchaseBAAPI::Checkout failed, Could not find corresponding offer."));
		return;
	}

	if (PendingPurchaseDelegate.IsSet())
	{
		CallDelegateError(TEXT("FOnlinePurchaseBAAPI::Checkout failed, there was another purchase in progress."));
		return;
	}

	PendingPurchaseDelegate = Delegate;

	TWeakPtr<FOnlinePurchaseBAAPI, ESPMode::ThreadSafe> WeakMe = AsShared();
	const FUniqueNetIdBAAPI& BAAPIUserId = static_cast<const FUniqueNetIdBAAPI&>(UserId);

	BAAPISubsystem.ExecuteNextTick([BAAPIUserId, BAAPIOffer, WeakMe]
	{
		FOnlinePurchaseBAAPIPtr StrongThis = WeakMe.Pin();
		if (StrongThis.IsValid())
		{
			StrongThis->CheckoutSuccessfully(BAAPIUserId, BAAPIOffer);
		}
	});
}

void FOnlinePurchaseBAAPI::CheckoutSuccessfully(const FUniqueNetIdBAAPI& UserId, TSharedPtr<FOnlineStoreOffer> Offer)
{
	// Cache this receipt
	TArray<FPurchaseReceipt>& UserReceipts = UserFakeReceipts.FindOrAdd(UserId);
	FPurchaseReceipt& PurchaseReceipt = UserReceipts.Emplace_GetRef();
	PurchaseReceipt.AddReceiptOffer(MakeReceiptOfferEntry(UserId, Offer->OfferId, Offer->Title.ToString()));

	check(PendingPurchaseDelegate.IsSet());

	// Have a pending purchase
	FOnPurchaseCheckoutComplete Delegate = MoveTemp(PendingPurchaseDelegate.GetValue());
	PendingPurchaseDelegate.Reset();
	PendingPurchaseFailTime.Reset();

	// Finish pending purchase
	Delegate.ExecuteIfBound(FOnlineError(true), MakeShared<FPurchaseReceipt>(PurchaseReceipt));
}

void FOnlinePurchaseBAAPI::FinalizePurchase(const FUniqueNetId& UserId, const FString& ReceiptId)
{
	const FUniqueNetIdBAAPI& BAAPIUserId = static_cast<const FUniqueNetIdBAAPI&>(UserId);
	TArray<FPurchaseReceipt>* UserReceipts = UserFakeReceipts.Find(BAAPIUserId);
	if (UserReceipts)
	{
		for (const FPurchaseReceipt& UserReceipt : *UserReceipts)
		{
			for (const FPurchaseReceipt::FReceiptOfferEntry& ReceiptOffer : UserReceipt.ReceiptOffers)
			{
				if (ReceiptOffer.OfferId == ReceiptId)
				{
					UE_LOG_ONLINE(Log, TEXT("Consumption of Entitlement %s completed was successful"), *ReceiptId);
					return;
				}
			}
		}
	}

	UE_LOG_ONLINE(Error, TEXT("Didn't find receipt with id %s"), *ReceiptId);
}

void FOnlinePurchaseBAAPI::RedeemCode(const FUniqueNetId& UserId, const FRedeemCodeRequest& RedeemCodeRequest, const FOnPurchaseRedeemCodeComplete& Delegate)
{
	TWeakPtr<FOnlinePurchaseBAAPI, ESPMode::ThreadSafe> WeakMe = AsShared();
	const FUniqueNetIdBAAPI& BAAPIUserId = static_cast<const FUniqueNetIdBAAPI&>(UserId);

	BAAPISubsystem.ExecuteNextTick([BAAPIUserId, WeakMe, RedeemCodeRequest, Delegate]
	{
		FOnlinePurchaseBAAPIPtr StrongThis = WeakMe.Pin();
		if (StrongThis.IsValid())
		{
			UE_LOG_ONLINE(Log, TEXT("FOnlinePurchaseBAAPI::RedeemCode redeemed successfully"));

			// Cache this receipt
			TArray<FPurchaseReceipt>& UserReceipts = StrongThis->UserFakeReceipts.FindOrAdd(BAAPIUserId);
			FPurchaseReceipt& PurchaseReceipt = UserReceipts.Emplace_GetRef();
			PurchaseReceipt.AddReceiptOffer(MakeReceiptOfferEntry(BAAPIUserId, RedeemCodeRequest.Code, RedeemCodeRequest.Code));

			Delegate.ExecuteIfBound(FOnlineError(true), MakeShared<FPurchaseReceipt>(PurchaseReceipt));
		}
	});
}

void FOnlinePurchaseBAAPI::QueryReceipts(const FUniqueNetId& UserId, bool bRestoreReceipts, const FOnQueryReceiptsComplete& Delegate)
{
	const FUniqueNetIdBAAPI& BAAPIUserId = static_cast<const FUniqueNetIdBAAPI&>(UserId);
	if (!BAAPIUserId.IsValid())
	{
		BAAPISubsystem.ExecuteNextTick([Delegate]
		{
			UE_LOG_ONLINE(Error, TEXT("FOnlinePurchaseBAAPI::QueryReceipts user is invalid"));

			Delegate.ExecuteIfBound(FOnlineError(TEXT("User is invalid")));
		});
		return;
	}

	BAAPISubsystem.ExecuteNextTick([Delegate]
	{
		Delegate.ExecuteIfBound(FOnlineError(true));
	});
}

void FOnlinePurchaseBAAPI::GetReceipts(const FUniqueNetId& UserId, TArray<FPurchaseReceipt>& OutReceipts) const
{
	const FUniqueNetIdBAAPI& BAAPIUserId = static_cast<const FUniqueNetIdBAAPI&>(UserId);

	const TArray<FPurchaseReceipt>* FoundReceipts = UserFakeReceipts.Find(BAAPIUserId);
	if (FoundReceipts == nullptr)
	{
		OutReceipts.Empty();
	}
	else
	{
		OutReceipts = *FoundReceipts;
	}
}

void FOnlinePurchaseBAAPI::FinalizeReceiptValidationInfo(const FUniqueNetId& UserId, FString& InReceiptValidationInfo, const FOnFinalizeReceiptValidationInfoComplete& Delegate)
{

}
