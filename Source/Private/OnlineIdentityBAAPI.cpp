// Copyright Epic Games, Inc. All Rights Reserved.

#include "OnlineIdentityBAAPI.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Guid.h"
#include "Misc/OutputDeviceRedirector.h"
#include "OnlineSubsystemBAAPI.h"
#include "IPAddress.h"
#include "SocketSubsystem.h"
#include "OnlineError.h"
#include "JsonObjectConverter.h"
#include "Types/LoginDataPicker.h"
#include "Interfaces/IHttpRequest.h"
#include "HttpModule.h"
#include "Interfaces/IHttpBase.h"
#include "BAMultiplayerSubsystem.h"

bool FUserOnlineAccountBAAPI::GetAuthAttribute(const FString& AttrName, FString& OutAttrValue) const
{
	const FString* FoundAttr = AdditionalAuthData.Find(AttrName);
	if (FoundAttr != NULL)
	{
		OutAttrValue = *FoundAttr;
		return true;
	}
	return false;
}

bool FUserOnlineAccountBAAPI::GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const
{
	const FString* FoundAttr = UserAttributes.Find(AttrName);
	if (FoundAttr != NULL)
	{
		OutAttrValue = *FoundAttr;
		return true;
	}
	return false;
}

bool FUserOnlineAccountBAAPI::SetUserAttribute(const FString& AttrName, const FString& AttrValue)
{
	const FString* FoundAttr = UserAttributes.Find(AttrName);
	if (FoundAttr == NULL || *FoundAttr != AttrValue)
	{
		UserAttributes.Add(AttrName, AttrValue);
		return true;
	}
	return false;
}

FString FOnlineIdentityBAAPI::GenerateRandomUserId(int32 LocalUserNum)
{
	FString HostName;
	if (!ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetHostName(HostName))
	{
		// could not get hostname, use address
		bool bCanBindAll;
		TSharedPtr<class FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBindAll);
		HostName = Addr->ToString(false);
	}

	bool bUseStableBAAPIId = bForceStableBAAPIId;
	FString UserSuffix;

	if (bAddUserNumToBAAPIId)
	{
		UserSuffix = FString::Printf(TEXT("-%d"), LocalUserNum);
	}

#if !(UE_BUILD_SHIPPING && WITH_EDITOR)
	if (GIsFirstInstance && !GIsEditor)
	{
		// If we're outside the editor and know this is the first instance, use the system login id
		bUseStableBAAPIId = true;
	}
#endif

	if (bUseStableBAAPIId)
	{
		// Use a stable id possibly with a user num suffix
		return FString::Printf(TEXT("%s-%s%s"), *HostName, *FPlatformMisc::GetLoginId().ToUpper(), *UserSuffix);
	}

	// If we're not the first instance (or in the editor), return truly random id
	return FString::Printf(TEXT("%s-%s%s"), *HostName, *FGuid::NewGuid().ToString(), *UserSuffix);
}

bool FOnlineIdentityBAAPI::Login(int32 LocalUserNum, const FOnlineAccountCredentials& AccountCredentials)
{

	UBAMultiplayerSubsystem* BAMultiplayerSubsystem = UBAMultiplayerSubsystem::GetInstance();

	if (!BAMultiplayerSubsystem || !BAMultiplayerSubsystem->IsValidLowLevel())
	{
		FString ErrorStr = FString::Printf(TEXT("BAMultiplayerSubsystem not found!"));
		UE_LOG_ONLINE_IDENTITY(Warning, TEXT("Login request failed. %s"), *ErrorStr);
		TriggerOnLoginCompleteDelegates(LocalUserNum, false, *FUniqueNetIdBAAPI::EmptyId(), ErrorStr);
		return false;
	}

	FBAIdentityLoginRequest Request;
	Request.Username = AccountCredentials.Id;
	Request.Password = AccountCredentials.Token;

	FBAIdentityOnLoginDelegate Delegate;

	Delegate.BindLambda
	(
		[this, LocalUserNum, Request](FBAIdentityLoginResponse Response)
		{
			if (Response.Success)
			{
				TSharedPtr<FUserOnlineAccountBAAPI> UserAccountPtr = MakeShareable(new FUserOnlineAccountBAAPI(Request.Username));
				UserAccountPtr->UserAttributes.Add(USER_ATTR_ID, Request.Username);
				// update/add cached entry for user
				UserAccounts.Add(UserAccountPtr->GetUserId(), UserAccountPtr.ToSharedRef());
				// keep track of user ids for local users
				UserIds.Add(LocalUserNum, UserAccountPtr->GetUserId());
				TriggerOnLoginCompleteDelegates(LocalUserNum, true, *UserAccountPtr->GetUserId(), TEXT(""));
			}
			else
			{
				FString ErrorStr = FString::Printf(TEXT("Login request failed!"));
				UE_LOG_ONLINE_IDENTITY(Warning, TEXT("Login request failed. %s"), *ErrorStr);
				TriggerOnLoginCompleteDelegates(LocalUserNum, false, *FUniqueNetIdBAAPI::EmptyId(), ErrorStr);
			}
		}
	);

	BAMultiplayerSubsystem->IdentityLogin(Request, Delegate);
	return true;
}

bool FOnlineIdentityBAAPI::Logout(int32 LocalUserNum)
{
	FUniqueNetIdPtr UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		// remove cached user account
		UserAccounts.Remove(UserId.ToSharedRef());
		// remove cached user id
		UserIds.Remove(LocalUserNum);
		// not async but should call completion delegate anyway
		TriggerOnLogoutCompleteDelegates(LocalUserNum, true);

		return true;
	}
	else
	{
		UE_LOG_ONLINE_IDENTITY(Warning, TEXT("No logged in user found for LocalUserNum=%d."),
			LocalUserNum);
		TriggerOnLogoutCompleteDelegates(LocalUserNum, false);
	}
	return false;
}

bool FOnlineIdentityBAAPI::AutoLogin(int32 LocalUserNum)
{
	FString LoginStr;
	FString PasswordStr;
	FString TypeStr;

	FParse::Value(FCommandLine::Get(), TEXT("AUTH_LOGIN="), LoginStr);
	FParse::Value(FCommandLine::Get(), TEXT("AUTH_PASSWORD="), PasswordStr);
	FParse::Value(FCommandLine::Get(), TEXT("AUTH_TYPE="), TypeStr);

	bool bEnableWarning = LoginStr.Len() > 0 || PasswordStr.Len() > 0 || TypeStr.Len() > 0;

	if (!LoginStr.IsEmpty())
	{
		if (!PasswordStr.IsEmpty())
		{
			if (!TypeStr.IsEmpty())
			{
				return Login(LocalUserNum, FOnlineAccountCredentials(TypeStr, LoginStr, PasswordStr));
			}
			else if (bEnableWarning)
			{
				UE_LOG_ONLINE_IDENTITY(Warning, TEXT("AutoLogin missing AUTH_TYPE=<type>."));
			}
		}
		else if (bEnableWarning)
		{
			UE_LOG_ONLINE_IDENTITY(Warning, TEXT("AutoLogin missing AUTH_PASSWORD=<password>."));
		}
	}
	else if (!bRequireLoginCredentials)
	{
		// Act like a console and login with empty auth
		return Login(LocalUserNum, FOnlineAccountCredentials());
	}
	else if (bEnableWarning)
	{
		UE_LOG_ONLINE_IDENTITY(Warning, TEXT("AutoLogin missing AUTH_LOGIN=<login id>."));
	}

	return false;
}

TSharedPtr<FUserOnlineAccount> FOnlineIdentityBAAPI::GetUserAccount(const FUniqueNetId& UserId) const
{
	TSharedPtr<FUserOnlineAccount> Result;

	if (const TSharedRef<FUserOnlineAccountBAAPI>* FoundUserAccount = UserAccounts.Find(UserId.AsShared()))
	{
		Result = *FoundUserAccount;
	}

	return Result;
}

TArray<TSharedPtr<FUserOnlineAccount> > FOnlineIdentityBAAPI::GetAllUserAccounts() const
{
	TArray<TSharedPtr<FUserOnlineAccount> > Result;

	for (TUniqueNetIdMap<TSharedRef<FUserOnlineAccountBAAPI>>::TConstIterator It(UserAccounts); It; ++It)
	{
		Result.Add(It.Value());
	}

	return Result;
}

FUniqueNetIdPtr FOnlineIdentityBAAPI::GetUniquePlayerId(int32 LocalUserNum) const
{
	const FUniqueNetIdPtr* FoundId = UserIds.Find(LocalUserNum);
	if (FoundId != NULL)
	{
		return *FoundId;
	}
	return NULL;
}

FUniqueNetIdPtr FOnlineIdentityBAAPI::CreateUniquePlayerId(uint8* Bytes, int32 Size)
{
	if (Bytes != NULL && Size > 0)
	{
		FString StrId(Size, (TCHAR*)Bytes);
		return FUniqueNetIdBAAPI::Create(StrId);
	}
	return NULL;
}

FUniqueNetIdPtr FOnlineIdentityBAAPI::CreateUniquePlayerId(const FString& Str)
{
	return FUniqueNetIdBAAPI::Create(Str);
}

ELoginStatus::Type FOnlineIdentityBAAPI::GetLoginStatus(int32 LocalUserNum) const
{
	FUniqueNetIdPtr UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		return GetLoginStatus(*UserId);
	}
	return ELoginStatus::NotLoggedIn;
}

ELoginStatus::Type FOnlineIdentityBAAPI::GetLoginStatus(const FUniqueNetId& UserId) const
{
	TSharedPtr<FUserOnlineAccount> UserAccount = GetUserAccount(UserId);
	if (UserAccount.IsValid() &&
		UserAccount->GetUserId()->IsValid())
	{
		if (bForceOfflineMode)
		{
			return ELoginStatus::UsingLocalProfile;
		}
		return ELoginStatus::LoggedIn;
	}
	return ELoginStatus::NotLoggedIn;
}

FString FOnlineIdentityBAAPI::GetPlayerNickname(int32 LocalUserNum) const
{
	FUniqueNetIdPtr UniqueId = GetUniquePlayerId(LocalUserNum);
	if (UniqueId.IsValid())
	{
		return UniqueId->ToString();
	}

	return TEXT("BAAPIUser");
}

FString FOnlineIdentityBAAPI::GetPlayerNickname(const FUniqueNetId& UserId) const
{
	return UserId.ToString();
}

FString FOnlineIdentityBAAPI::GetAuthToken(int32 LocalUserNum) const
{
	FUniqueNetIdPtr UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		TSharedPtr<FUserOnlineAccount> UserAccount = GetUserAccount(*UserId);
		if (UserAccount.IsValid())
		{
			return UserAccount->GetAccessToken();
		}
	}
	return FString();
}

void FOnlineIdentityBAAPI::RevokeAuthToken(const FUniqueNetId& UserId, const FOnRevokeAuthTokenCompleteDelegate& Delegate)
{
	UE_LOG_ONLINE_IDENTITY(Display, TEXT("FOnlineIdentityBAAPI::RevokeAuthToken not implemented"));
	FUniqueNetIdRef UserIdRef(UserId.AsShared());
	BAAPISubsystem->ExecuteNextTick([UserIdRef, Delegate]()
		{
			Delegate.ExecuteIfBound(*UserIdRef, FOnlineError(FString(TEXT("RevokeAuthToken not implemented"))));
		});
}

FOnlineIdentityBAAPI::FOnlineIdentityBAAPI(FOnlineSubsystemBAAPI* InSubsystem)
	: BAAPISubsystem(InSubsystem)
{
	// Read configuration variables for emulating other login systems
	GConfig->GetBool(TEXT("OnlineSubsystemBAAPI"), TEXT("bAutoLoginAtStartup"), bAutoLoginAtStartup, GEngineIni);
	GConfig->GetBool(TEXT("OnlineSubsystemBAAPI"), TEXT("bRequireLoginCredentials"), bRequireLoginCredentials, GEngineIni);
	GConfig->GetBool(TEXT("OnlineSubsystemBAAPI"), TEXT("bAddUserNumToBAAPIId"), bAddUserNumToBAAPIId, GEngineIni);
	GConfig->GetBool(TEXT("OnlineSubsystemBAAPI"), TEXT("bForceStableBAAPIId"), bForceStableBAAPIId, GEngineIni);
	GConfig->GetBool(TEXT("OnlineSubsystemBAAPI"), TEXT("bForceOfflineMode"), bForceOfflineMode, GEngineIni);

	if (FParse::Param(FCommandLine::Get(), TEXT("StableBAAPIID")))
	{
		bForceStableBAAPIId = true;
	}

	if (bAutoLoginAtStartup)
	{
		// autologin the 0-th player
		Login(0, FOnlineAccountCredentials(TEXT("DummyType"), TEXT("DummyUser"), TEXT("DummyId")));
	}
}

FOnlineIdentityBAAPI::~FOnlineIdentityBAAPI()
{
}

void FOnlineIdentityBAAPI::GetUserPrivilege(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, const FOnGetUserPrivilegeCompleteDelegate& Delegate)
{
	if (bForceOfflineMode && Privilege == EUserPrivileges::CanPlayOnline)
	{
		Delegate.ExecuteIfBound(UserId, Privilege, (uint32)EPrivilegeResults::NetworkConnectionUnavailable);
	}
	Delegate.ExecuteIfBound(UserId, Privilege, (uint32)EPrivilegeResults::NoFailures);
}

FPlatformUserId FOnlineIdentityBAAPI::GetPlatformUserIdFromUniqueNetId(const FUniqueNetId& UniqueNetId) const
{
	for (int i = 0; i < MAX_LOCAL_PLAYERS; ++i)
	{
		auto CurrentUniqueId = GetUniquePlayerId(i);
		if (CurrentUniqueId.IsValid() && (*CurrentUniqueId == UniqueNetId))
		{
			return GetPlatformUserIdFromLocalUserNum(i);
		}
	}

	return PLATFORMUSERID_NONE;
}

FString FOnlineIdentityBAAPI::GetAuthType() const
{
	return TEXT("");
}
