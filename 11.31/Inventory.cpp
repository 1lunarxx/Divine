#include "pch.h"
#include "Inventory.h"
#include "Utils.h"

UFortWorldItem* Inventory::CreateItem(AFortInventory* WorldInventory, UFortItemDefinition* ItemDefinition, int32 Count, int32 LoadedAmmo)
{
	UFortWorldItem* WorldItem = (UFortWorldItem*)ItemDefinition->CreateTemporaryItemInstanceBP(Count, 0);

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

FFortItemEntry* Inventory::GetItemEntry(AFortInventory* WorldInventory, UFortItemDefinition* ItemDefinition)
{
	FFortItemEntry* ItemEntry = NULL;

	for (FFortItemEntry& ReplicatedEntry : WorldInventory->Inventory.ReplicatedEntries)
	{
		if (ReplicatedEntry.ItemDefinition == ItemDefinition)
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
		if (ReplicatedEntry.ItemGuid == ItemEntry->ItemGuid)
		{
			ReplicatedEntry = *ItemEntry;
			ReplicatedEntry.bIsDirty = false;

			WorldInventory->Inventory.MarkItemDirty(*ItemEntry);

			WorldInventory->bRequiresLocalUpdate = true;
			WorldInventory->HandleInventoryLocalUpdate();

			if (ItemEntry)
				WorldInventory->Inventory.MarkItemDirty(*ItemEntry);
			else
				WorldInventory->Inventory.MarkArrayDirty();
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

void Inventory::AttemptToStack(AFortInventory* WorldInventory, UFortItemDefinition* ItemDefinition, int32 Count, int32 LoadedAmmo)
{
	FFortItemEntry* ItemEntry = GetItemEntry(WorldInventory, ItemDefinition); 

	if (ItemEntry != NULL)
	{
		int32 MaxStackSize = ItemDefinition->MaxStackSize;

		int32 TotalCount = ItemEntry->Count + Count;
		int32 OverStack = TotalCount - MaxStackSize;

		if (TotalCount <= MaxStackSize)
		{
			ItemEntry->Count += Count;

			WorldInventory->Inventory.MarkItemDirty(*ItemEntry);
			WorldInventory->HandleInventoryLocalUpdate();
		}
		else
		{
			ItemEntry->Count += MaxStackSize - ItemEntry->Count;

			WorldInventory->Inventory.MarkItemDirty(*ItemEntry);
			WorldInventory->HandleInventoryLocalUpdate();

			AttemptToStack(WorldInventory, ItemDefinition, Count, LoadedAmmo);
		}
	}
	else
	{
		CreateItem(WorldInventory, ItemDefinition, Count, LoadedAmmo);
	}
}

void Inventory::GiveItem(AFortInventory* WorldInventory, UFortItemDefinition* ItemDefinition, int32 Count, int32 LoadedAmmo)
{
	if (ItemDefinition->IsStackable())
		AttemptToStack(WorldInventory, ItemDefinition, Count, LoadedAmmo);
	else
		CreateItem(WorldInventory, ItemDefinition, Count, LoadedAmmo);
}