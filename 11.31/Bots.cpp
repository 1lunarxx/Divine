#include "pch.h"
#include "Bots.h"
#include "Utils.h"
#include "Inventory.h"

void Bots::InitializeMMRInfos(AFortAthenaMutator_Bots* BotMutator)
{
	*(uint8*)(__int64(BotMutator) + 0x428) = 1;

	return Originals::InitializeMMRInfos(BotMutator);
}

void Bots::OnPawnSpawned(AFortAthenaMutator_Bots* BotMutator, AFortPlayerPawn* PlayerPawn)
{
	Originals::OnPawnSpawned(BotMutator, PlayerPawn);

	AFortAthenaAIBotController* BotController = Cast<AFortAthenaAIBotController>(PlayerPawn->Controller);

	if (BotController == NULL)
		return;

	for (UFortAthenaAIBotDigestedSkillSet* SkillSet : BotController->DigestedBotSkillSets)
	{
		if (UFortAthenaAIBotAimingDigestedSkillSet* AimingSkill = Cast<UFortAthenaAIBotAimingDigestedSkillSet>(SkillSet))
			BotController->CacheAimingDigestedSkillSet = AimingSkill;

		if (UFortAthenaAIBotHarvestDigestedSkillSet* HarvestSkill = Cast<UFortAthenaAIBotHarvestDigestedSkillSet>(SkillSet))
			BotController->CacheHarvestDigestedSkillSet = HarvestSkill;

		if (UFortAthenaAIBotInventoryDigestedSkillSet* InventorySkill = Cast<UFortAthenaAIBotInventoryDigestedSkillSet>(SkillSet))
			BotController->CacheInventoryDigestedSkillSet = InventorySkill;

		if (UFortAthenaAIBotMovementDigestedSkillSet* MovementSkill = Cast<UFortAthenaAIBotMovementDigestedSkillSet>(SkillSet))
			BotController->CacheMovementSkillSet = MovementSkill;

		if (UFortAthenaAIBotPerceptionDigestedSkillSet* PerceptionSkill = Cast<UFortAthenaAIBotPerceptionDigestedSkillSet>(SkillSet))
			BotController->CachePerceptionDigestedSkillSet = PerceptionSkill;

		if (UFortAthenaAIBotPlayStyleDigestedSkillSet* PlayStyleSkill = Cast<UFortAthenaAIBotPlayStyleDigestedSkillSet>(SkillSet))
			BotController->CachePlayStyleSkillSet = PlayStyleSkill;
	}

	int j = 0;

	for (int i = 0; i < BotController->Inventory->Inventory.ItemInstances.Num(); i++)
	{
		if (j > 6)
			break;

		UFortItemDefinition* ItemDef = BotController->Inventory->Inventory.ItemInstances[i]->GetItemDefinitionBP();

		if (ItemDef->bInventorySizeLimited)
		{
			FFortBotInventoryInfo& SlotItem = BotController->SlotItems[j];

			SlotItem.FortItem = BotController->Inventory->Inventory.ItemInstances[i];
			SlotItem.ItemDefinition = ItemDef;
			SlotItem.ItemType = ItemDef->ItemType;
			SlotItem.WeaponType = AFortWeapon::FindWeaponTypeForStats(ItemDef);

			j++;
		}
	}
}

void Bots::InitializeInventory(AFortAthenaAIBotController* BotController, UFortAthenaAIBotInventoryItems* OverrideStartupInventory)
{
	if (BotController->Inventory == NULL)
	{
		BotController->Inventory = Utils::SpawnActor<AFortInventory>(FVector(), FRotator(), AFortInventory::StaticClass(), BotController);

		AddStartingItemsToInventory(BotController);
	}

	return Originals::InitializeInventory(BotController, OverrideStartupInventory);
}

void Bots::AddStartingItemsToInventory(AFortAthenaAIBotController* BotController)
{
	for (UFortItemDefinition* Item : BotController->StartupInventory->Items)
	{
		UFortWorldItem* WorldItem = CreateItem(BotController, Item, 1);
	}
}

void Bots::InitializeCosmeticLoadout(AFortAthenaAIBotController* BotController, void* a2, void* a3)
{
	Originals::InitializeCosmeticLoadout(BotController, a2, a3);

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(BotController->PlayerState);

	if (PlayerState == NULL)
		return;

	static UDataTable* PhoebeEmotes = Utils::StaticLoadObject<UDataTable>(TEXT("/Game/Athena/AI/Phoebe/Cosmetic/Pheobe_Cosmetic_Emotes.Pheobe_Cosmetic_Emotes"));

	for (const auto& [RowName, RowValue] : PhoebeEmotes->RowMap)
	{
		FFortBotCosmeticItemDataTableRow* TableRow = (FFortBotCosmeticItemDataTableRow*)RowValue;

		if (TableRow != NULL)
		{
			UAthenaDanceItemDefinition* Dance = (UAthenaDanceItemDefinition*)UKismetSystemLibrary::GetObjectFromPrimaryAssetId(TableRow->PrimaryAssetId);

			if (Dance)
				BotController->CosmeticLoadoutBC.Dances.Add(Dance);
		}
	}
}

void Bots::OnPossessedPawnDied(AFortAthenaAIBotController* BotController, AActor* DamagedActor, float Damage, AFortPlayerControllerAthena* InstigatedBy, AActor* DamageCauser, FVector HitLocation, UPrimitiveComponent* PrimitiveComponent, FName BoneName, FVector Momentum)
{
	Originals::OnPossessedPawnDied(BotController, DamagedActor, Damage, InstigatedBy, DamageCauser, HitLocation, PrimitiveComponent, BoneName, Momentum);

	if (InstigatedBy == NULL)
		return;

	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);

	if (GameState == NULL)
		return;

	AFortPlayerStateAthena* KillerPlayerState = Cast<AFortPlayerStateAthena>(InstigatedBy->PlayerState);

	if (KillerPlayerState == NULL)
		return;

	if (BotController->Inventory)
	{
		for (FFortItemEntry& Entry : BotController->Inventory->Inventory.ReplicatedEntries)
		{
			if (UFortWeaponRangedItemDefinition* WeaponItemDefinition = Cast<UFortWeaponRangedItemDefinition>(Entry.ItemDefinition))
			{
				Inventory::SpawnPickup(BotController->PlayerBotPawn->K2_GetActorLocation(), &Entry, Entry.Count, BotController->PlayerBotPawn, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::PlayerElimination);
			}
		}
	}

	if (AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(BotController->PlayerState))
	{
		PlayerState->PawnDeathLocation = BotController->PlayerBotPawn->K2_GetActorLocation();

		if (BotController->bIsAnAthenaGameParticipant)
		{
			PlayerState->DeathInfo.bDBNO = BotController->PlayerBotPawn->bWasDBNOOnDeath;
			PlayerState->DeathInfo.DeathLocation = BotController->PlayerBotPawn->K2_GetActorLocation();
			PlayerState->DeathInfo.DeathTags = BotController->PlayerBotPawn->DeathTags;
			PlayerState->DeathInfo.Distance = (InstigatedBy->Pawn && BotController->PlayerBotPawn) ? InstigatedBy->Pawn->GetDistanceTo(BotController->PlayerBotPawn) : 0.f;
			PlayerState->DeathInfo.DeathCause = AFortPlayerStateAthena::ToDeathCause(FGameplayTagContainer(), PlayerState->DeathInfo.bDBNO);
			PlayerState->DeathInfo.FinisherOrDowner = KillerPlayerState ? KillerPlayerState : PlayerState;

			PlayerState->DeathInfo.bInitialized = true;

			PlayerState->OnRep_DeathInfo();

			if (KillerPlayerState != PlayerState)
			{
				KillerPlayerState->KillScore++;
				KillerPlayerState->OnRep_Kills();

				KillerPlayerState->ClientReportKill(PlayerState);

				if (KillerPlayerState->PlayerTeam)
				{
					for (auto& TeamMember : KillerPlayerState->PlayerTeam->TeamMembers)
					{
						AFortPlayerStateAthena* TeamMemberState = Cast<AFortPlayerStateAthena>(TeamMember->PlayerState);

						if (TeamMemberState == NULL)
							continue;

						TeamMemberState->TeamKillScore++;
						TeamMemberState->OnRep_TeamKillScore();

						TeamMemberState->ClientReportTeamKill(TeamMemberState->TeamKillScore);
					}
				}
			}
		}
	}
}

void Bots::EnterAircraft(AFortAthenaAIBotController* BotController, AFortAthenaAircraft* InAircraft)
{
	if (UFortControllerComponent_Aircraft* AircraftComponent = Cast<UFortControllerComponent_Aircraft>(BotController->GetComponentByClass(UFortControllerComponent_Aircraft::StaticClass())))
	{
		AircraftComponent->EnterAircraft(InAircraft);
	}

	if (UFortServerBotManagerAthena* ServerBotManager = BotController->CachedBotManager)
	{
		const float ThankBusDriverMinTime = UFortScalableFloatUtils::GetValueAtLevel(ServerBotManager->ThankBusDriverMinTime, 0);
		const float ThankBusDriverMaxTime = UFortScalableFloatUtils::GetValueAtLevel(ServerBotManager->ThankBusDriverMaxTime, 0);

		const float ThankBusDriverTime = UKismetMathLibrary::RandomIntegerInRange(ThankBusDriverMinTime, ThankBusDriverMaxTime);

		UKismetSystemLibrary::K2_SetTimer(BotController, TEXT("ThankBusDriver"), ThankBusDriverTime, false, 0.0, 0.0f);
	}

	if (UBlackboardComponent* Blackboard = BotController->Blackboard)
	{
		const FName IsInBusKey = UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_IsInBus");
		const FName HasEverJumpedFromBusKey = UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_HasEverJumpedFromBusKey");
		const FName HasEverJumpedFromBusAndLandedKey = UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_HasEverJumpedFromBusAndLandedKey");

		Blackboard->SetValueAsBool(IsInBusKey, true);
		Blackboard->SetValueAsBool(HasEverJumpedFromBusKey, false);
		Blackboard->SetValueAsBool(HasEverJumpedFromBusAndLandedKey, false);
	}

	if (AFortPlayerStateZone* PlayerState = Cast<AFortPlayerStateZone>(BotController->PlayerState))
	{
		PlayerState->bHasEverSkydivedFromBus = 0;
		PlayerState->bHasEverSkydivedFromBusAndLanded = 0;
	}
}

UFortWorldItem* Bots::CreateItem(AFortAthenaAIBotController* BotController, UFortItemDefinition* ItemDefinition, int32 Count)
{
	UFortWorldItem* NewItem = Cast<UFortWorldItem>(ItemDefinition->CreateTemporaryItemInstanceBP(Count, 0));

	if (NewItem == NULL)
		return NULL;

	NewItem->OwnerInventory = BotController->Inventory;

	TWeakObjectPtr<AFortInventory> WeakInventory{};
	WeakInventory.ObjectIndex = BotController->Inventory->Index;
	WeakInventory.ObjectSerialNumber = UObject::GObjects->GetSerialByIndex(BotController->Inventory->Index);

	NewItem->ItemEntry.ParentInventory = WeakInventory;

	NewItem->ItemEntry.ItemDefinition = ItemDefinition;
	NewItem->ItemEntry.Count = Count;
	NewItem->ItemEntry.LoadedAmmo = 30;

	for (int32 Index = 0; Index < 6; Index++)
	{
		FFortBotInventoryInfo& SlotItem = BotController->SlotItems[Index];

		if (SlotItem.FortItem == NULL || SlotItem.ItemDefinition == NULL)
		{
			SlotItem.FortItem = NewItem;
			SlotItem.ItemDefinition = ItemDefinition;
			SlotItem.ItemType = ItemDefinition->ItemType;
			SlotItem.WeaponType = AFortWeapon::FindWeaponTypeForStats(ItemDefinition);

			break;
		}
	}

	BotController->Inventory->Inventory.ItemInstances.Add(NewItem);	BotController->Inventory->Inventory.ReplicatedEntries.Add(NewItem->ItemEntry);

	BotController->Inventory->Inventory.MarkItemDirty(NewItem->ItemEntry);

	BotController->Inventory->HandleInventoryLocalUpdate();

	return NewItem;
}

AFortAthenaAIBotController* Bots::GetSquadLeader(AFortAthenaAIBotController* BotController)
{
	return BotController;
}

UFortServerBotManagerAthena* Bots::GetServerBotManager()
{
	AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);
	return GameMode->ServerBotManager;
}

void Bots::Setup()
{
	Utils::Hook(InSDKUtils::GetImageBase() + 0x165CB70, OnPawnSpawned, (void**)&Originals::OnPawnSpawned);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x12AB760, OnPossessedPawnDied, (void**)&Originals::OnPossessedPawnDied);

	Utils::Hook(InSDKUtils::GetImageBase() + 0x12A7810, InitializeInventory, (void**)&Originals::InitializeInventory);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x1296DF0, InitializeCosmeticLoadout, (void**)&Originals::InitializeCosmeticLoadout);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x1654BB0, InitializeMMRInfos, (void**)&Originals::InitializeMMRInfos);

	Utils::Hook(InSDKUtils::GetImageBase() + 0x12A46A0, GetSquadLeader);

	Utils::Rel32(InSDKUtils::GetImageBase() + 0x146E480, GetServerBotManager);
}