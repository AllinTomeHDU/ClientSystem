// Copyright: Jichao Luo

#pragma once

#include "CoreMinimal.h"
#include "ClientWorkType.generated.h"

class ACharacter;


USTRUCT(BlueprintType)
struct CLIENTSYSTEM_API FClientUserInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Account;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Password;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString IPCountry;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Platform;
};

USTRUCT(BlueprintType)
struct CLIENTSYSTEM_API FClientUserAssets
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Rank;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpiritStone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ImmortalJade;
};

