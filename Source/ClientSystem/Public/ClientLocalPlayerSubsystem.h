// Copyright: Jichao Luo

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "DS_NetChannel/Core/NetChannelType.h"
#include "Core/ClientWorkType.h"
#include "ClientLocalPlayerSubsystem.generated.h"

class FNetChannelManager;
class FNetChannelBase;

DECLARE_MULTICAST_DELEGATE(FClientJoinGateCompleteDelegate);

DECLARE_MULTICAST_DELEGATE(FClientAccountAlreadyExitsDelegate);
DECLARE_MULTICAST_DELEGATE_OneParam(FClientRegisterCompleteDelegate, const bool);

DECLARE_MULTICAST_DELEGATE(FClientAbsentAccountDelegate);
DECLARE_MULTICAST_DELEGATE_OneParam(FClientLoginCompleteDelegate, const bool);


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
	FClientLoginCompleteDelegate		OnClientLoginComplete;

	UFUNCTION(BlueprintCallable)
	void RegisterAccount(const FClientUserInfo& InClientUserInfo);

	UFUNCTION(BlueprintCallable)
	void LoginServer(const FString& Account, const FString& Password, const FString& Platform = TEXT("NULL"));

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	void ClientTick();

	void TryRegister();

	void TryLoginGate();
	void TryLoginHall();

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

	float LoginWaitTime{ 0.f };
	float LoginWaitTimeTreshold{ 5.f };

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
