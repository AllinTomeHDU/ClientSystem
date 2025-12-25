// Copyright: Jichao Luo


#include "ClientWidgetBase.h"
#include "ClientLocalPlayerSubsystem.h"



void UClientWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (auto LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController())
	{
		ClientSubsystem = LocalPlayer->GetSubsystem<UClientLocalPlayerSubsystem>();
		if (IsValid(ClientSubsystem))
		{
			ClientSubsystem->OnClientTmpMessage.AddUObject(this, &ThisClass::OnTmpMessageBP);
			ClientSubsystem->OnClientJoinGateComplete.AddUObject(this, &ThisClass::OnJoinGateComplete);

			ClientSubsystem->OnClientAccountAlreadyExits.AddUObject(this, &ThisClass::OnClientAccountAlreadyExits);
			ClientSubsystem->OnClientRegisterComplete.AddUObject(this, &ThisClass::OnClientRegisterComplete);

			ClientSubsystem->OnClientAbsentAccount.AddUObject(this, &ThisClass::OnClientAbsentAccount);
			ClientSubsystem->OnClientAbnormalAccount.AddUObject(this, &ThisClass::OnClientAbnormalAccountBP);
			ClientSubsystem->OnClientIncorrectPassword.AddUObject(this, &ThisClass::OnClientIncorrectPassword);
			ClientSubsystem->OnClientVerificationError.AddUObject(this, &ThisClass::OnClientVerificationError);
			ClientSubsystem->OnClientLoginComplete.AddUObject(this, &ThisClass::OnClientLoginComplete);
		}
	}
}

void UClientWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();

	OnClientWidgetDestruct.Broadcast();
}

void UClientWidgetBase::OnTmpMessage(const FString& Message, const FString& MessageType)
{
	OnTmpMessageBP(Message, MessageType);
}

void UClientWidgetBase::OnJoinGateComplete(const bool bIsSuccess)
{
	OnJoinGateCompleteBP(bIsSuccess);
}

void UClientWidgetBase::OnClientAccountAlreadyExits()
{
	OnClientAccountAlreadyExitsBP();
}

void UClientWidgetBase::OnClientRegisterComplete(const bool bIsSuccess)
{
	OnClientRegisterCompleteBP(bIsSuccess);
}

void UClientWidgetBase::OnClientAbsentAccount()
{
	OnClientAbsentAccountBP();
}

void UClientWidgetBase::OnClientAbnormalAccount()
{
	OnClientAbnormalAccountBP();
}

void UClientWidgetBase::OnClientIncorrectPassword()
{
	OnClientIncorrectPasswordBP();
}

void UClientWidgetBase::OnClientVerificationError()
{
	OnClientVerificationErrorBP();
}

void UClientWidgetBase::OnClientLoginComplete(const bool bIsSuccess)
{
	OnClientLoginCompleteBP(bIsSuccess);
}
