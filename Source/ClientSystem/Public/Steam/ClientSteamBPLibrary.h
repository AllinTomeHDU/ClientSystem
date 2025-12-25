// Copyright: Jichao Luo

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ClientSteamBPLibrary.generated.h"

/**
 * 
 */
UCLASS()
class CLIENTSYSTEM_API UClientSteamBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "ClientSystem")
	static FString GetSteamID();

	UFUNCTION(BlueprintCallable, Category = "ClientSystem")
	static FString GetAccountID();

	UFUNCTION(BlueprintCallable, Category = "ClientSystem")
	static FString GetPersonaName();

	UFUNCTION(BlueprintCallable, Category = "ClientSystem")
	static FString GetIPCountry();
};
