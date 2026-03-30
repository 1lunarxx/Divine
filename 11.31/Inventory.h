#pragma once
#include "pch.h"

class Inventory
{
public:
	static UFortWorldItem* CreateItem(AFortInventory*, UFortItemDefinition*, int32 = 1, int32 = 0);

	static AFortPickupAthena* SpawnPickup(FVector, FFortItemEntry*, int, AFortPlayerPawn*, EFortPickupSourceTypeFlag, EFortPickupSpawnSource, bool, bool);

	static UFortWorldItem* GetWorldItem(AFortInventory*, FGuid&);

	static FFortItemEntry* GetItemEntry(AFortInventory*, FGuid&);
	static FFortItemEntry* GetItemEntry(AFortInventory*, UClass*);

	static void RemoveItem(AFortInventory*, FGuid&);
	static void UpdateEntry(AFortInventory*, FFortItemEntry*);
};