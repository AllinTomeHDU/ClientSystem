#include "ClientObjectController.h"


void UClientObjectController::Init()
{
	Super::Init();
}

void UClientObjectController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void UClientObjectController::Close()
{
	Super::Close();
}

/*
* 这个地方可以根据具体项目的不同写不同的业务逻辑
*/
void UClientObjectController::RecvProtocol(uint32 InProtocol)
{
	Super::RecvProtocol(InProtocol);

	switch (InProtocol)
	{
		case P_ResponseUserAssets:
		{
			FUserAssets UserAssets;
			NETCHANNEL_PROTOCOLS_RECV(P_ResponseUserAssets, UserAssets);

			ClientUserAssets.Rank = UTF8_TO_TCHAR(UserAssets.Rank);
			ClientUserAssets.SpiritStone = UserAssets.SpiritStone;
			ClientUserAssets.ImmortalJade = UserAssets.ImmortalJade;
			break;
		}
		case P_FailedRequestUserAssets:
		{
			FString ErrorMsg;
			NETCHANNEL_PROTOCOLS_RECV(P_FailedRequestUserAssets, ErrorMsg);
			UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMsg);
			break;
		}
	}


}
