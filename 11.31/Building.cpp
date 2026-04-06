#include "pch.h"
#include "Building.h"
#include "Utils.h"
#include "Inventory.h"

void Building::ServerBeginEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* Building)
{
	if (Building == NULL || !Building->bPlayerPlaced)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	Building->SetEditingPlayer(PlayerState);

	FFortItemEntry* ItemEntry = Inventory::GetItemEntry(PlayerController->WorldInventory, UFortEditToolItemDefinition::StaticClass());

	if (ItemEntry == NULL)
		return;

	UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemEntry->ItemDefinition);

	if (WeaponItemDefinition == NULL)
		return;

	PlayerController->MyFortPawn->EquipWeaponDefinition(WeaponItemDefinition, ItemEntry->ItemGuid);

	if (AFortWeap_EditingTool* EditingTool = Cast<AFortWeap_EditingTool>(PlayerController->MyFortPawn->CurrentWeapon))
	{
		EditingTool->EditActor = Building;
		EditingTool->OnRep_EditActor();
	}
}

void Building::ServerEditBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* Building, TSubclassOf<ABuildingSMActor> NewClass, uint8 RotationIterations, bool bMirrored)
{
	if (Building == NULL || NewClass == NULL || Building->bDestroyed || Building->EditingPlayer != PlayerController->PlayerState)
		return;

	Building->SetEditingPlayer(NULL);

	ABuildingSMActor* NewBuilding = Building->ReplaceBuildingActor(EBuildingReplacementType::BRT_Edited, NewClass, Building->CurrentBuildingLevel, RotationIterations, bMirrored, PlayerController);

	if (NewBuilding != NULL)
		NewBuilding->bPlayerPlaced = true;

	return Originals::ServerEditBuildingActor(PlayerController, Building, NewClass, RotationIterations, bMirrored);
}

void Building::ServerEndEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* Building)
{
	if (PlayerController->MyFortPawn == NULL || Building == NULL || Building->bDestroyed || Building->EditingPlayer != PlayerController->PlayerState)
		return;

	FFortItemEntry* ItemEntry = Inventory::GetItemEntry(PlayerController->WorldInventory, UFortEditToolItemDefinition::StaticClass());

	if (ItemEntry == NULL)
		return;

	UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemEntry->ItemDefinition);

	if (WeaponItemDefinition == NULL)
		return;

	Building->SetEditingPlayer(NULL);

	AFortWeap_EditingTool* EditingTool = Cast<AFortWeap_EditingTool>(PlayerController->MyFortPawn->EquipWeaponDefinition(WeaponItemDefinition, ItemEntry->ItemGuid));

	if (EditingTool != NULL)
	{
		EditingTool->EditActor = NULL;
		EditingTool->OnRep_EditActor();
	}
}

void Building::ServerCreateBuildingActor(AFortPlayerControllerAthena* PlayerController, FCreateBuildingActorData BuildingActorData)
{
	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);

	if (GameState == NULL)
		return;

	UClass* BuildingClass = PlayerController->BroadcastRemoteClientInfo->RemoteBuildableClass.Get();

	if (BuildingClass == NULL)
		return;

	EFortBuildPreviewMarkerOptionalAdjustment MarkerOptionalAdjustment;

	TArray<ABuildingActor*> ExistingBuildings;

	EFortStructuralGridQueryResults QueryResults = GameState->StructuralSupportSystem->CanAddBuildingActorClassToGrid(UWorld::GetWorld(), BuildingClass, BuildingActorData.BuildLoc, BuildingActorData.BuildRot, BuildingActorData.bMirrored, &ExistingBuildings, &MarkerOptionalAdjustment, false);

	if (QueryResults == EFortStructuralGridQueryResults::CanAdd)
	{
		for (ABuildingActor* ExistingBuilding : ExistingBuildings)
		{
			ExistingBuilding->K2_DestroyActor();
		}

		ABuildingSMActor* Building = Utils::SpawnActor<ABuildingSMActor>(BuildingActorData.BuildLoc, BuildingActorData.BuildRot, BuildingClass, PlayerController);

		if (Building == NULL)
			return;

		Building->InitializeKismetSpawnedBuildingActor(Building, PlayerController, true);
		Building->bPlayerPlaced = true;

		if (AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState))
		{
			Building->PlacedByPlayer(PlayerState);

			Building->SetTeam(PlayerState->TeamIndex);
			Building->OnRep_Team();
		}
	}
}

void Building::OnDamageServer(ABuildingSMActor* BuildingSMActor, float Damage, FGameplayTagContainer DamageTags, FVector Momentum, FHitResult HitInfo, AFortPlayerControllerAthena* InstigatedBy, AActor* DamageCauser, FGameplayEffectContextHandle EffectContext)
{
	if (!InstigatedBy || BuildingSMActor->bPlayerPlaced || BuildingSMActor->GetHealth() == 1)
		return Originals::OnDamageServer(BuildingSMActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	if (!DamageCauser || !DamageCauser->IsA(AFortWeapon::StaticClass()) || !((AFortWeapon*)DamageCauser)->WeaponData->IsA(UFortWeaponMeleeItemDefinition::StaticClass()))
		return Originals::OnDamageServer(BuildingSMActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	UFortResourceItemDefinition* Resource = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingSMActor->ResourceType);
	if (!Resource)
		return Originals::OnDamageServer(BuildingSMActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	int32 ResourceCount = (int32)std::round(UKismetMathLibrary::RandomIntegerInRange(11, 33) / (BuildingSMActor->GetMaxHealth() / Damage));
	if (ResourceCount > 0)
	{
		FFortItemEntry* ItemEntry = InstigatedBy->WorldInventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& Entry)
			{
				return Entry.ItemDefinition == Resource;
			});

		if (ItemEntry)
		{
			ItemEntry->Count += ResourceCount;
			if (ItemEntry->Count > 500)
			{
				ItemEntry->Count = 500;
			}

			Inventory::UpdateItemEntry(InstigatedBy->WorldInventory, ItemEntry);
		}
		else
		{
			if (ResourceCount > 500)
			{
				ResourceCount = 500;
			}

			Inventory::CreateItem(InstigatedBy->WorldInventory, Resource, ResourceCount);
		}
	}

	InstigatedBy->ClientReportDamagedResourceBuilding(BuildingSMActor, BuildingSMActor->ResourceType, ResourceCount, false, Damage == 100.f);
	return Originals::OnDamageServer(BuildingSMActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
}


void Building::Setup()
{
	Utils::Virtual<AFortPlayerControllerAthena>(0x1190 / 8, ServerEditBuildingActor, (void**)&Originals::ServerEditBuildingActor);
	Utils::Virtual<AFortPlayerControllerAthena>(0x11B8 / 8, ServerBeginEditingBuildingActor);
	Utils::Virtual<AFortPlayerControllerAthena>(0x11A8 / 8, ServerEndEditingBuildingActor);
	Utils::Virtual<AFortPlayerControllerAthena>(0x1180 / 8, ServerCreateBuildingActor);

	Utils::Hook(ImageBase + 0x2176A20, OnDamageServer, (void**)&Originals::OnDamageServer);
}