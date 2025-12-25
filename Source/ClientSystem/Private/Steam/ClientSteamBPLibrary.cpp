// Copyright: Jichao Luo


#include "Steam/ClientSteamBPLibrary.h"
#include "Steamworks/Steamv151/sdk/public/steam/steam_api.h"


FString UClientSteamBPLibrary::GetSteamID()
{
	if (!SteamUser()) FString();

	return FString::Printf(TEXT("%llu"), SteamUser()->GetSteamID().ConvertToUint64());
}

FString UClientSteamBPLibrary::GetAccountID()
{
	if (!SteamUser()) FString();

	return FString::Printf(TEXT("%u"), SteamUser()->GetSteamID().GetAccountID());
}

FString UClientSteamBPLibrary::GetPersonaName()
{
	if (!SteamFriends()) FString();

	return UTF8_TO_TCHAR(SteamFriends()->GetPersonaName());
}

FString UClientSteamBPLibrary::GetIPCountry()
{
	if (!SteamUtils()) return FString();

	return UTF8_TO_TCHAR(SteamUtils()->GetIPCountry());
}
