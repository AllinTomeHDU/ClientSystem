#pragma once

#include "CoreMinimal.h"
#include "Core/ClientWorkType.h"
#include "DS_NetChannel/UObject/NetChannelController.h"
#include "DS_NetChannel/Core/NetChannelProtocols.h"
#include "DS_NetChannel/Core/NetChannelType.h"
#include "ClientObjectController.generated.h"


UCLASS()
class CLIENTSYSTEM_API UClientObjectController : public UNetChannelController
{
	GENERATED_BODY()

	struct FUserAssets
	{
		FUserAssets() : SpiritStone(0), ImmortalJade(0)
		{
			FMemory::Memset(Rank, 0, sizeof(Rank));
		}
		char Rank[20];
		uint32 SpiritStone;
		uint32 ImmortalJade;
	};

public:


protected:
	virtual void Init() override;
	virtual void Tick(float DeltaTime) override;
	virtual void Close() override;
	virtual void RecvProtocol(uint32 InProtocol) override;

private:
	FClientUserAssets ClientUserAssets;

public:
	FORCEINLINE FClientUserAssets GetClientUserAssets() const { return ClientUserAssets; }
};