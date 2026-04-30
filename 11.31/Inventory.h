#pragma once
#include "pch.h"

class Inventory
{
public:
	static UFortWorldItem* CreateItem(AFortInventory*, UFortItemDefinition*, int32 = 1, int32 = 0);

	static AFortPickupAthena* SpawnPickup(FVector, FFortItemEntry*, int32, AFortPlayerPawn*, EFortPickupSourceTypeFlag, EFortPickupSpawnSource);

	static FFortRangedWeaponStats* GetStats(UFortWeaponItemDefinition*);

	static UFortWorldItem* GetWorldItem(AFortInventory*, FGuid&);

	static FFortItemEntry* GetItemEntry(AFortInventory*, FGuid&);
	static FFortItemEntry* GetItemEntry(AFortInventory*, UClass*);
	static FFortItemEntry* GetItemEntry(AFortInventory*, UFortItemDefinition*);

	static void UpdateItemEntry(AFortInventory*, FFortItemEntry*);
	static void RemoveItem(AFortInventory*, FGuid&);

	static void AttemptToStack(AFortInventory*, UFortItemDefinition*, int32 = 1, int32 = 0);
	static void GiveItem(AFortInventory*, UFortItemDefinition*, int32 = 1, int32 = 0);
};