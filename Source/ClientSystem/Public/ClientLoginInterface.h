// Copyright: Jichao Luo

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/ClientWorkType.h"
#include "ClientLoginInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UClientLoginInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CLIENTSYSTEM_API IClientLoginInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetPlatform(const FString& InPlatform);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetRegisterInfo(const FClientUserInfo& UserInfo);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetLoginInfo(const FString& Account, const FString& Password, const FString& Platform);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ClickLogin();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ClickRegister();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void AccountAlreadyExits();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RegisterComplete(const bool bSuccess);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void AbsentAccount();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void LoginComplete(const bool bSuccess);
};
