#include "pch.h"
#include "Navigation.h"
#include "Utils.h"

void Navigation::InitializeForWorld(UNavigationSystemV1* NavigationSystem, UWorld* World, EFNavigationSystemRunMode Mode)
{
	NavigationSystem->bAutoCreateNavigationData = true;
	NavigationSystem->bSpawnNavDataInNavBoundsLevel = true;
	NavigationSystem->SupportedAgentsMask.bSupportsAgent3 = 1;

	return Originals::InitializeForWorld(NavigationSystem, World, Mode);
}

void Navigation::CreateAndConfigureNavigationSystem(UAthenaNavSystemConfig* NavigationSystemConfig, UWorld* World)
{
	NavigationSystemConfig->bPrioritizeNavigationAroundSpawners = true;
	NavigationSystemConfig->bSupportRuntimeNavmeshDisabling = false;
	NavigationSystemConfig->bAutoSpawnMissingNavData = true;
	NavigationSystemConfig->bAllowAutoRebuild = true;
	NavigationSystemConfig->bUsesStreamedInNavLevel = true;

	return Originals::CreateAndConfigureNavigationSystem(NavigationSystemConfig, World);
}

void Navigation::Setup()
{
	Utils::Hook(InSDKUtils::GetImageBase() + 0x3D875A0, CreateAndConfigureNavigationSystem, (void**)&Originals::CreateAndConfigureNavigationSystem);
	Utils::Virtual<UAthenaNavSystem>(0x50, InitializeForWorld, (void**)&Originals::InitializeForWorld);
}