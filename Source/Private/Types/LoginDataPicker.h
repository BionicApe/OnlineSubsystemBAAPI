// Created by Bionic Ape. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "LoginDataPicker.generated.h"


USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBAAPI_API FLoginDataPicker
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Username;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//FString Email;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Password;
};