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

#include "OnlineSubsystem.h"
#include "Steam/ClientSteamBPLibrary.h"


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

	InitUserInfo();
}

void UClientLocalPlayerSubsystem::InitUserInfo()
{
	PersonaUserInfo.Platform = IOnlineSubsystem::Get()->GetSubsystemName().ToString();
	if (PersonaUserInfo.Platform == TEXT("STEAM"))
	{
		PersonaUserInfo.Account = UClientSteamBPLibrary::GetSteamID();
		PersonaUserInfo.Name = UClientSteamBPLibrary::GetPersonaName();
		PersonaUserInfo.IPCountry = UClientSteamBPLibrary::GetIPCountry();
	}
	else
	{
		PersonaUserInfo.Name = TEXT("UnrealEditor");
	}
}

void UClientLocalPlayerSubsystem::ClientTick()
{
	Client->Tick(TickDelta);

	// 避免因长时间收不到服务器发送的信号而卡死在登录界面
	if (!bIsLoginComplete && bEnableConnectWait)
	{
		ConnectWaitTime += TickDelta;
		if (ConnectWaitTime > ConnectWaitTimeTreshold)
		{
			OnClientLoginComplete.Broadcast(false);
			bIsLoginComplete = true;
		}
	}
}

UClientObjectController* UClientLocalPlayerSubsystem::GetClientObjectController()
{
	return Client ? Cast<UClientObjectController>(Client->GetController()) : nullptr;
}

void UClientLocalPlayerSubsystem::RegisterAccount(const FClientUserInfo& InClientUserInfo)
{
	PersonaUserInfo = InClientUserInfo;
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
			bEnableConnectWait = true;
		}
	}
}

void UClientLocalPlayerSubsystem::LoginGate(const FString& Account, const FString& Password)
{
	PersonaUserInfo.Account = Account;
	PersonaUserInfo.Password = Password;
	if (Client && Client->GetController())
	{
		if (auto Channel = Client->GetController()->GetChannel())
		{
			NETCHANNEL_PROTOCOLS_SEND(P_Login, PersonaUserInfo.Account, PersonaUserInfo.Password);
			bEnableConnectWait = true;
		}
	}
}

void UClientLocalPlayerSubsystem::LoginHall()
{
	if (Client && Client->GetController())
	{
		if (auto Channel = Client->GetController()->GetChannel())
		{
			NETCHANNEL_PROTOCOLS_SEND(P_Login, PersonaUserInfo.Account);
			bEnableConnectWait = true;
		}
	}
}

void UClientLocalPlayerSubsystem::JoinGateCallback(bool bWasSuccess)
{
	// Steam、Epic等平台，可在通过认证之后自动登录，其他的平台可能需要输入账户密码登录
	if (PersonaUserInfo.Platform == TEXT("STEAM"))
	{
		LoginGate(PersonaUserInfo.Account, PersonaUserInfo.Password);
		OnClientTmpMessage.Broadcast(TEXT("Logining in..."), TEXT("Success"));
	}
	else
	{
		// 手动输入账号
		OnClientJoinGateComplete.Broadcast(bWasSuccess);
		bEnableConnectWait = false;
	}
	ReconnectTimes = 0;
	ConnectWaitTime = 0;

	// Join失败，原因目前仅可能是Client和Server版本不一致
}

void UClientLocalPlayerSubsystem::JoinHallCallback(bool bWasSuccess)
{
	if (bWasSuccess)
	{
		LoginHall();
		ReconnectTimes = 0;
		ConnectWaitTime = 0;
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

void UClientLocalPlayerSubsystem::RecvGateCallback(uint32 ProtocolNumber, FNetChannelBase* Channel)
{
	switch (ProtocolNumber)
	{
		/*
		* 注册结果
		*/
		case P_RegisterSuccess:
		{
			if (PersonaUserInfo.Platform == TEXT("STEAM"))
			{
				LoginGate(PersonaUserInfo.Account, PersonaUserInfo.Password);
				OnClientTmpMessage.Broadcast(TEXT("Register Success, Logining in..."), TEXT("Success"));
			}
			else
			{
				OnClientRegisterComplete.Broadcast(true);
			}
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
				[this]() { RegisterAccount(PersonaUserInfo); },
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

		/*
		* 登录结果
		*/
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
			ConnectWaitTime = 0;
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
				[this]() { LoginGate(PersonaUserInfo.Account, PersonaUserInfo.Password); }, 
				1.f, 
				false
			);
			break;
		}
		case P_AbsentAccount:
		{
			if (PersonaUserInfo.Platform == TEXT("STEAM"))
			{
				RegisterAccount(PersonaUserInfo);
				
			}
			else
			{
				OnClientAbsentAccount.Broadcast();
			}
			break;
		}
		case P_AbnormalAccount:
		{
			OnClientAbnormalAccount.Broadcast();
			break;
		}
		case P_IncorrectPassword:
		{
			OnClientIncorrectPassword.Broadcast();
			break;
		}
		case P_VerificationError:
		{
			OnClientVerificationError.Broadcast();
			break;
		}
	}
	bEnableConnectWait = false;
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
				[this]() { LoginHall(); },
				1.f,
				false
			);
			break;
		}
	}
	bEnableConnectWait = false;
}


