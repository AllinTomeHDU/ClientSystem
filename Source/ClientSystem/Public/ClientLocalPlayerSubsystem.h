// Copyright: Jichao Luo

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "DS_NetChannel/Core/NetChannelType.h"
#include "Definition/ClientWorkType.h"
#include "ClientLocalPlayerSubsystem.generated.h"

class FNetChannelManager;
class FNetChannelBase;
class UClientObjectController;

DECLARE_MULTICAST_DELEGATE_OneParam(FClientJoinGateCompleteDelegate, const bool);

DECLARE_MULTICAST_DELEGATE(FClientAccountAlreadyExitsDelegate);
DECLARE_MULTICAST_DELEGATE_OneParam(FClientRegisterCompleteDelegate, const bool);

DECLARE_MULTICAST_DELEGATE(FClientAbsentAccountDelegate);
DECLARE_MULTICAST_DELEGATE(FClientAbnormalAccountDelegate);
DECLARE_MULTICAST_DELEGATE(FClientIncorrectPasswordDelegate);
DECLARE_MULTICAST_DELEGATE(FClientVerificationErrorDelegate);
DECLARE_MULTICAST_DELEGATE_OneParam(FClientLoginCompleteDelegate, const bool);

DECLARE_MULTICAST_DELEGATE_TwoParams(FClientTmpMessageDelegate, const FString&, const FString&);


/**
 * 
 */
UCLASS()
class CLIENTSYSTEM_API UClientLocalPlayerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	FClientJoinGateCompleteDelegate		OnClientJoinGateComplete;

	FClientAccountAlreadyExitsDelegate	OnClientAccountAlreadyExits;
	FClientRegisterCompleteDelegate		OnClientRegisterComplete;

	FClientAbsentAccountDelegate		OnClientAbsentAccount;
	FClientAbnormalAccountDelegate		OnClientAbnormalAccount;
	FClientIncorrectPasswordDelegate	OnClientIncorrectPassword;
	FClientVerificationErrorDelegate	OnClientVerificationError;
	FClientLoginCompleteDelegate		OnClientLoginComplete;

	FClientTmpMessageDelegate			OnClientTmpMessage;

public:
	UFUNCTION(BlueprintPure)
	UClientObjectController* GetClientObjectController();

	UFUNCTION(BlueprintCallable)
	void RegisterAccount(const FClientUserInfo& InClientUserInfo);

	UFUNCTION(BlueprintCallable)
	void LoginGate(const FString& Account, const FString& Password);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void InitUserInfo();
	void ClientTick();

private:
	void LoginHall();

	void JoinGateCallback(bool bWasSuccess);
	void JoinHallCallback(bool bWasSuccess);

	void RecvGateCallback(uint32 ProtocolNumber, FNetChannelBase* Channel);
	void RecvHallCallback(uint32 ProtocolNumber, FNetChannelBase* Channel);

private:
	FNetChannelManager* Client;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FClientUserInfo PersonaUserInfo;

	FTimerHandle ClientHeartBeatTimeHandle;
	float TickDelta{ 0.1f };

	int32 ReconnectTimes{ 0 };
	int32 ReconnectTimesThreshold{ 3 };

	bool bEnableConnectWait{ true };
	float ConnectWaitTime{ 0.f };
	float ConnectWaitTimeTreshold{ 5.f };

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsLoginComplete{ false };

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsLoginSuccess{ false };

public:
	FORCEINLINE FNetChannelManager* GetClient() const { return Client; }
	FORCEINLINE FClientUserInfo GetClientUserInfo() const { return PersonaUserInfo; }

	FORCEINLINE bool IsLoginComplete() const { return bIsLoginComplete; }
	FORCEINLINE bool IsLoginSuccess() const { return bIsLoginSuccess; }
};
