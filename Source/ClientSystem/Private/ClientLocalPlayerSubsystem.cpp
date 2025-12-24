// Copyright: Jichao Luo


#include "ClientLocalPlayerSubsystem.h"
#include "ClientObjectController.h"
#include "DS_NetChannel/NetChannelGlobalInfo.h"
#include "DS_NetChannel/NetChannelManager.h"
#include "DS_NetChannel/Core/NetChannelProtocols.h"
#include "DS_NetChannel/Connection/Base/NetConnectionBase.h"
#include "DS_NetChannel/Channel/NetChannelBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"



void UClientLocalPlayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FNetConfigInfo NetConfigInfo;
	FNetChannelGlobalInfo::Get()->SetConfigInfo(NetConfigInfo);
	FNetChannelBase::SimpleControllerDelegate.BindLambda(
		[]()->UClass* { return UClientObjectController::StaticClass(); }
	);

	Client = FNetChannelManager::CreateNetChannelManager(ENetLinkState::Connect, ENetSocketType::UDP);
	if (!Client || !Client->Init())
	{
		delete Client;
		OnClientLoginComplete.Broadcast(false);
		return;
	}
	if (Client->GetController())
	{
		Client->GetController()->JoinDelegate.AddUObject(this, &ThisClass::JoinGateCallback);
		Client->GetController()->RecvDelegate.AddUObject(this, &ThisClass::RecvGateCallback);
	}

	if (GetWorld())
	{
		/*
		* 1.接收服务器的信息
		* 2.发送心跳协议
		* 3.LocalConnect、MainChannel、ClientController更新Tick逻辑
		*/
		GetWorld()->GetTimerManager().SetTimer(
			ClientHeartBeatTimeHandle,
			this,
			&ThisClass::ClientTick,
			TickDelta,
			true
		);
	}
}

void UClientLocalPlayerSubsystem::ClientTick()
{
	Client->Tick(TickDelta);

	//if (!bIsLoginComplete)
	//{
	//	LoginWaitTime += TickDelta;
	//	if (LoginWaitTime > LoginWaitTimeTreshold)
	//	{
	//		OnClientLoginComplete.Broadcast(false);
	//		bIsLoginComplete = true;
	//	}
	//}
}

void UClientLocalPlayerSubsystem::RegisterAccount(const FClientUserInfo& InClientUserInfo)
{
	PersonaUserInfo = InClientUserInfo;
	TryRegister();
}

void UClientLocalPlayerSubsystem::LoginServer(const FString& Account, const FString& Password, const FString& Platform)
{
	PersonaUserInfo.Account = Account;
	PersonaUserInfo.Password = Password;
	PersonaUserInfo.Platform = Platform;
	TryLoginGate();
}


void UClientLocalPlayerSubsystem::TryLoginGate()
{
	if (Client && Client->GetController())
	{
		if (auto Channel = Client->GetController()->GetChannel())
		{
			NETCHANNEL_PROTOCOLS_SEND(P_Login, PersonaUserInfo.Account, PersonaUserInfo.Password);
		}
	}
}

void UClientLocalPlayerSubsystem::TryRegister()
{
	if (Client && Client->GetController())
	{
		if (auto Channel = Client->GetController()->GetChannel())
		{
			NETCHANNEL_PROTOCOLS_SEND(
				P_Register,
				PersonaUserInfo.Account,
				PersonaUserInfo.Password,
				PersonaUserInfo.Name,
				PersonaUserInfo.IPCountry,
				PersonaUserInfo.Platform
			);
		}
	}
}

void UClientLocalPlayerSubsystem::TryLoginHall()
{
	if (Client && Client->GetController())
	{
		if (auto Channel = Client->GetController()->GetChannel())
		{
			NETCHANNEL_PROTOCOLS_SEND(P_Login, PersonaUserInfo.Account);
		}
	}
}

void UClientLocalPlayerSubsystem::JoinGateCallback(bool bWasSuccess)
{
	if (bWasSuccess)
	{
		OnClientJoinGateComplete.Broadcast();
		ReconnectTimes = 0;
		LoginWaitTime = 0;
	}
	else
	{
		if (++ReconnectTimes > ReconnectTimesThreshold)
		{
			OnClientLoginComplete.Broadcast(false);
			return;
		}

		if (Client && Client->GetLocalConnection().IsValid())
		{
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle,
				[this]() { Client->GetLocalConnection()->Verify(); },
				1.f,
				false
			);
		}
	}
}

void UClientLocalPlayerSubsystem::JoinHallCallback(bool bWasSuccess)
{
	if (bWasSuccess)
	{
		TryLoginHall();
		ReconnectTimes = 0;
		LoginWaitTime = 0;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Client Join Gate Failed, try join again..."));
		if (Client && Client->GetLocalConnection().IsValid())
		{
			if (++ReconnectTimes > ReconnectTimesThreshold)
			{
				OnClientLoginComplete.Broadcast(false);
				return;
			}

			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle,
				[this]() { Client->GetLocalConnection()->Verify(); },
				1.f,
				false
			);
		}
	}
}

void UClientLocalPlayerSubsystem::RecvGateCallback(uint32 ProtocolNumber, FNetChannelBase* Channel)
{
	switch (ProtocolNumber)
	{
		case P_RegisterSuccess:
		{
			OnClientRegisterComplete.Broadcast(true);
			break;
		}
		case P_RegisterFailure:
		{
			FString ErrorMsg;
			NETCHANNEL_PROTOCOLS_RECV(P_LoginFailure, ErrorMsg);
			UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMsg);

			// 可能是网络原因导致注册失败，尝试重新注册
			if (++ReconnectTimes > ReconnectTimesThreshold)
			{
				OnClientRegisterComplete.Broadcast(false);
				break;
			}
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle,
				[this]() { TryRegister(); },
				1.f,
				false
			);
			break;
		}
		case P_AccountAlreadyExits:
		{
			OnClientAccountAlreadyExits.Broadcast();
			break;
		}
		case P_LoginSuccess:
		{
			FNetServerInfo HallServerInfo;
			NETCHANNEL_PROTOCOLS_RECV(P_LoginSuccess, HallServerInfo);

			Channel->CloseConnect();
			if (Client->Bind(HallServerInfo.Addr))
			{
				Client->GetController()->JoinDelegate.AddUObject(this, &ThisClass::JoinHallCallback);
				Client->GetController()->RecvDelegate.AddUObject(this, &ThisClass::RecvHallCallback);
			}
			ReconnectTimes = 0;
			LoginWaitTime = 0;
			break;
		}
		case P_LoginFailure:
		{
			FString ErrorMsg;
			NETCHANNEL_PROTOCOLS_RECV(P_LoginFailure, ErrorMsg);
			UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMsg);

			if (++ReconnectTimes > ReconnectTimesThreshold)
			{
				OnClientLoginComplete.Broadcast(false);
				break;
			}
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle, 
				[this]() { TryLoginGate(); }, 
				1.f, 
				false
			);
			break;
		}
		case P_IncorrectPassword:
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString(TEXT("IncorrectPassword")));
			break;
		}
		case P_AbsentAccount:
		{
			OnClientAbsentAccount.Broadcast();
			break;
		}
		case P_AbnormalAccount:
		{
			break;
		}
		case P_VerificationError:
		{
			break;
		}
	}
}

void UClientLocalPlayerSubsystem::RecvHallCallback(uint32 ProtocolNumber, FNetChannelBase* Channel)
{
	switch (ProtocolNumber)
	{
		case P_LoginSuccess:
		{
			bIsLoginSuccess = true;
			OnClientLoginComplete.Broadcast(true);
			bIsLoginComplete = true;
			break;
		}
		case P_LoginFailure:
		{
			FString ErrorMsg;
			NETCHANNEL_PROTOCOLS_RECV(P_LoginFailure, ErrorMsg);
			UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMsg);

			if (++ReconnectTimes > ReconnectTimesThreshold)
			{
				OnClientLoginComplete.Broadcast(false);
				break;
			}
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle,
				[this]() { TryLoginHall(); },
				1.f,
				false
			);
			break;
		}
	}
}


