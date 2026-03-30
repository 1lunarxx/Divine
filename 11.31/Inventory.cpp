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

AFortPickupAthena* Inventory::SpawnPickup(FVector Location, FFortItemEntry* ItemEntry, int Count, AFortPlayerPawn* Pawn, EFortPickupSourceTypeFlag SourceTypeFlag, EFortPickupSpawnSource SpawnSource, bool bToss, bool bRandomRotation)
{
	AFortPickupAthena* NewPickup = Utils::SpawnActor<AFortPickupAthena>(Location);

	if (NewPickup == NULL)
		return NULL;

	if (ItemEntry == NULL)
		return NULL;

	NewPickup->bRandomRotation = bRandomRotation;

	if (ItemEntry->Level != -1)
		NewPickup->PrimaryPickupItemEntry.Level = ItemEntry->Level;

	NewPickup->PrimaryPickupItemEntry.ItemDefinition = ItemEntry->ItemDefinition;
	NewPickup->PrimaryPickupItemEntry.LoadedAmmo = ItemEntry->LoadedAmmo;
	NewPickup->PrimaryPickupItemEntry.Count = Count != -1 ? Count : ItemEntry->Count;
	NewPickup->OnRep_PrimaryPickupItemEntry();

	NewPickup->PawnWhoDroppedPickup = Pawn ? Pawn : nullptr;
	NewPickup->TossPickup(Location, Pawn ? Pawn : nullptr, -1, bToss, true, SourceTypeFlag, SpawnSource);

	if (SpawnSource != EFortPickupSpawnSource::Unset)
	{
		NewPickup->bTossedFromContainer = SpawnSource == EFortPickupSpawnSource::Chest || SpawnSource == EFortPickupSpawnSource::AmmoBox;
		NewPickup->OnRep_TossedFromContainer();
	}

	return NewPickup;
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