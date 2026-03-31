#include "pch.h"
#include "Inventory.h"
#include "Utils.h"

UFortWorldItem* Inventory::CreateItem(AFortInventory* WorldInventory, UFortItemDefinition* ItemDefinition, int32 Count, int32 LoadedAmmo)
{
	if (ItemDefinition == NULL)
		return NULL;

	UFortWorldItem* WorldItem = (UFortWorldItem*)(ItemDefinition->CreateTemporaryItemInstanceBP(Count, 0));

	if (WorldItem == NULL)
		return NULL;

	AFortPlayerController* PlayerController = Cast<AFortPlayerController>(WorldInventory->GetOwner() ? WorldInventory->GetOwner() : WorldInventory->GetInstigatorController());

	if (PlayerController == NULL)
		return NULL;

	WorldItem->OwnerInventory = PlayerController->WorldInventory;	
	WorldItem->SetOwningControllerForTemporaryItem(PlayerController);

	WorldItem->ItemEntry.ItemDefinition = ItemDefinition;
	WorldItem->ItemEntry.LoadedAmmo = LoadedAmmo;

	WorldInventory->Inventory.ItemInstances.Add(WorldItem);
	WorldInventory->Inventory.ReplicatedEntries.Add(WorldItem->ItemEntry);
	WorldInventory->Inventory.MarkItemDirty(WorldItem->ItemEntry);

	WorldInventory->HandleInventoryLocalUpdate();

	return WorldItem;
}

AFortPickupAthena* Inventory::SpawnPickup(FVector Location, FFortItemEntry* ItemEntry, int Count, AFortPlayerPawn* Pawn, EFortPickupSourceTypeFlag SourceTypeFlag, EFortPickupSpawnSource SpawnSource)
{
	AFortPickupAthena* Pickup = Utils::SpawnActor<AFortPickupAthena>(Location);

	if (Pickup == NULL)
		return NULL;

	if (ItemEntry->Level != -1)
		Pickup->PrimaryPickupItemEntry.Level = ItemEntry->Level;

	Pickup->PrimaryPickupItemEntry.ItemDefinition = ItemEntry->ItemDefinition;
	Pickup->PrimaryPickupItemEntry.LoadedAmmo = ItemEntry->LoadedAmmo;
	Pickup->PrimaryPickupItemEntry.Count = Count != -1 ? Count : ItemEntry->Count;

	Pickup->OnRep_PrimaryPickupItemEntry();

	Pickup->bRandomRotation = true;

	Pickup->PawnWhoDroppedPickup = Pawn ? Pawn : NULL;
	Pickup->TossPickup(Location, Pickup->PawnWhoDroppedPickup, -1, true, true, SourceTypeFlag, SpawnSource);

	if (SpawnSource != EFortPickupSpawnSource::Unset)
	{
		Pickup->bTossedFromContainer = SpawnSource == EFortPickupSpawnSource::Chest || SpawnSource == EFortPickupSpawnSource::AmmoBox;
		Pickup->OnRep_TossedFromContainer();
	}

	return Pickup;
}

FFortRangedWeaponStats* Inventory::GetStats(UFortWeaponItemDefinition* ItemDefinition)
{
	if (ItemDefinition->WeaponStatHandle.DataTable == NULL)
		return NULL;

	for (const auto& [RowName, RowValue] : ItemDefinition->WeaponStatHandle.DataTable->RowMap)
	{
		FFortRangedWeaponStats* WeaponStats = (FFortRangedWeaponStats*)RowValue;

		if (WeaponStats == NULL)
			continue;

		if (ItemDefinition->WeaponStatHandle.RowName == RowName)
		{
			return WeaponStats;
		}
	}

	return NULL;
}

UFortWorldItem* Inventory::GetWorldItem(AFortInventory* WorldInventory, FGuid& ItemGuid)
{
	UFortWorldItem* WorldItem = NULL;

	for (UFortWorldItem* ItemInstance : WorldInventory->Inventory.ItemInstances)
	{
		if (ItemInstance->ItemEntry.ItemGuid == ItemGuid)
		{
			WorldItem = ItemInstance;
			break;
		}
	}

	return WorldItem;
}

FFortItemEntry* Inventory::GetItemEntry(AFortInventory* WorldInventory, FGuid& ItemGuid)
{
	FFortItemEntry* ItemEntry = NULL;

	for (FFortItemEntry& ReplicatedEntry : WorldInventory->Inventory.ReplicatedEntries)
	{
		if (ReplicatedEntry.ItemGuid == ItemGuid)
		{
			ItemEntry = &ReplicatedEntry;
			break;
		}
	}

	return ItemEntry;
}

FFortItemEntry* Inventory::GetItemEntry(AFortInventory* WorldInventory, UClass* Class)
{
	FFortItemEntry* ItemEntry = NULL;

	for (FFortItemEntry& ReplicatedEntry : WorldInventory->Inventory.ReplicatedEntries)
	{
		if (ReplicatedEntry.ItemDefinition->Class == Class)
		{
			ItemEntry = &ReplicatedEntry;
			break;
		}
	}

	return ItemEntry;
}

void Inventory::UpdateItemEntry(AFortInventory* WorldInventory, FFortItemEntry* ItemEntry)
{
	if (ItemEntry->bIsReplicatedCopy)
	{
		ItemEntry->bIsDirty = false;

		WorldInventory->Inventory.MarkItemDirty(*ItemEntry);
		WorldInventory->bRequiresLocalUpdate = true;

		return;
	}

	for (FFortItemEntry& ReplicatedEntry : WorldInventory->Inventory.ReplicatedEntries)
	{
		// Safer way to check imo

		if (ReplicatedEntry.ItemGuid == ItemEntry->ItemGuid)
		{
			ReplicatedEntry = *ItemEntry;

			ReplicatedEntry.bIsDirty = false;

			WorldInventory->Inventory.MarkItemDirty(*ItemEntry);

			WorldInventory->bRequiresLocalUpdate = true;
			WorldInventory->HandleInventoryLocalUpdate();

			if (ItemEntry)
			{
				WorldInventory->Inventory.MarkItemDirty(*ItemEntry);
			}
			else
			{
				WorldInventory->Inventory.MarkArrayDirty();
			}
		}
	}
}

void Inventory::RemoveItem(AFortInventory* WorldInventory, FGuid& ItemGuid)
{
	for (int i = 0; i < WorldInventory->Inventory.ItemInstances.Num(); i++)
	{
		if (WorldInventory->Inventory.ItemInstances[i]->ItemEntry.ItemGuid == ItemGuid)
		{
			WorldInventory->Inventory.ItemInstances.Remove(i);
			break;
		}
	}

	for (int i = 0; i < WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
	{
		if (WorldInventory->Inventory.ReplicatedEntries[i].ItemGuid == ItemGuid)
		{
			WorldInventory->Inventory.ReplicatedEntries.Remove(i);
			break;
		}
	}

	WorldInventory->bRequiresLocalUpdate = true;

	WorldInventory->HandleInventoryLocalUpdate();

	WorldInventory->Inventory.MarkArrayDirty();
}