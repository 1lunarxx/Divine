#include "pch.h"
#include "Navigation.h"
#include "Utils.h"

void Navigation::InitializeForWorld(UNavigationSystemV1* NavigationSystem, UWorld* World, EFNavigationSystemRunMode Mode)
{
	NavigationSystem->bAutoCreateNavigationData = true;
	//NavigationSystem->bSpawnNavDataInNavBoundsLevel = true;

	return Originals::InitializeForWorld(NavigationSystem, World, Mode);
}

void Navigation::Setup()
{
	Utils::Virtual<UAthenaNavSystem>(0x50, InitializeForWorld, (void**)&Originals::InitializeForWorld);
}