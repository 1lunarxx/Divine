#pragma once
#include "pch.h"

class Inventory
{
public:
	static UFortWorldItem* CreateItem(AFortInventory*, UFortItemDefinition*, int32 = 1, int32 = 0);
	static AFortPickupAthena* SpawnPickup(FVector Location, FFortItemEntry* ItemEntry, int Count, AFortPlayerPawnAthena* Pawn, EFortPickupSourceTypeFlag SourceTypeFlag, EFortPickupSpawnSource SpawnSource, bool bToss, bool bRandomRotation);

	static FFortItemEntry* GetItemEntry(AFortInventory*, FGuid&);
};