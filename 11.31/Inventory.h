#pragma once
#include "pch.h"

class Inventory
{
public:
	static UFortWorldItem* CreateItem(AFortInventory*, UFortItemDefinition*, int32 = 1, int32 = 0);

	static AFortPickupAthena* SpawnPickup(FVector, FFortItemEntry*, int, AFortPlayerPawnAthena*, EFortPickupSourceTypeFlag, EFortPickupSpawnSource, bool, bool);

	static FFortItemEntry* GetItemEntry(AFortInventory*, FGuid&);
	static FFortItemEntry* GetItemEntry(AFortInventory*, UClass*);
};