#include "pch.h"
#include "Looting.h"
#include "Utils.h"

bool Looting::SpawnLoot(ABuildingContainer* BuildingContainer)
{
    BuildingContainer->bAlreadySearched = 1;
    BuildingContainer->OnRep_bAlreadySearched();

    BuildingContainer->SearchBounceData.SearchAnimationCount++;
    BuildingContainer->BounceContainer();

    return true;
}

void Looting::Setup()
{
    Utils::Hook(InSDKUtils::GetImageBase() + 0x174AAA0, SpawnLoot);
}