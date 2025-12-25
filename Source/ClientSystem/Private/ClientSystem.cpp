// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClientSystem.h"
#include "Steamworks/Steamv151/sdk/public/steam/steam_api.h"

#define LOCTEXT_NAMESPACE "FClientSystemModule"

void FClientSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	SteamAPI_Init();
}

void FClientSystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	SteamAPI_Shutdown();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FClientSystemModule, ClientSystem)