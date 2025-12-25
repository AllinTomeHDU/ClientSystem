// Copyright: Jichao Luo

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Definition/ClientWorkType.h"
#include "ClientWidgetBase.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnClientWidgetDestructDelegate);

/**
 * 
 */
UCLASS()
class CLIENTSYSTEM_API UClientWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY()
	class UClientLocalPlayerSubsystem* ClientSubsystem;

public:
	FOnClientWidgetDestructDelegate OnClientWidgetDestruct;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnTmpMessageBP(const FString& Message, const FString& MessageType);

	UFUNCTION(BlueprintImplementableEvent)
	void OnJoinGateCompleteBP(const bool bIsSuccess);

	UFUNCTION(BlueprintImplementableEvent)
	void OnClientAccountAlreadyExitsBP();

	UFUNCTION(BlueprintImplementableEvent)
	void OnClientRegisterCompleteBP(const bool bIsSuccess);

	UFUNCTION(BlueprintImplementableEvent)
	void OnClientAbsentAccountBP();

	UFUNCTION(BlueprintImplementableEvent)
	void OnClientAbnormalAccountBP();

	UFUNCTION(BlueprintImplementableEvent)
	void OnClientIncorrectPasswordBP();

	UFUNCTION(BlueprintImplementableEvent)
	void OnClientVerificationErrorBP();

	UFUNCTION(BlueprintImplementableEvent)
	void OnClientLoginCompleteBP(const bool bIsSuccess);

private:
	void OnTmpMessage(const FString& Message, const FString& MessageType);
	void OnJoinGateComplete(const bool bIsSuccess);

	void OnClientAccountAlreadyExits();
	void OnClientRegisterComplete(const bool bIsSuccess);

	void OnClientAbsentAccount();
	void OnClientAbnormalAccount();
	void OnClientIncorrectPassword();
	void OnClientVerificationError();
	void OnClientLoginComplete(const bool bIsSuccess);

private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FClientUserInfo ClientUserInfo;

public:
	FORCEINLINE FClientUserInfo GetClientUserInfo() const { return ClientUserInfo; }
	FORCEINLINE void SetClientUserInfo(const FClientUserInfo& InUserInfo) { ClientUserInfo = InUserInfo; }
};
 