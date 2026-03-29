#include "pch.h"
#include "Bots.h"
#include "Utils.h"
#include "Inventory.h"

void Bots::InitializeMMRInfos(AFortAthenaMutator_Bots* BotMutator)
{
	*(uint8*)((__int64)(BotMutator) + 0x428) = UKismetMathLibrary::RandomIntegerInRange(0, 30); // CachedHumanPlayerCount

	return Originals::InitializeMMRInfos(BotMutator);
}

void Bots::OnBotSpawned(AFortAthenaMutator_Bots* BotMutator, AFortPlayerPawn* PlayerPawn)
{
	AFortAthenaAIBotController* BotController = Cast<AFortAthenaAIBotController>(PlayerPawn->Controller);

	if (BotController == NULL)
		return Originals::OnBotSpawned(BotMutator, PlayerPawn);

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

		if (UFortAthenaAIBotLootingDigestedSkillSet* LootSkill = Cast<UFortAthenaAIBotLootingDigestedSkillSet>(SkillSet))
			BotController->CacheLootingSkillSet = LootSkill;
	}

	BotController->BotSkillSetClasses.Add(UFortAthenaAIBotWarmupSkillSet::StaticClass());

	BotController->BehaviorTree = Utils::StaticLoadObject<UBehaviorTree>("/Game/Athena/AI/Phoebe/BehaviorTrees/BT_Phoebe_Warmup.BT_Phoebe_Warmup");
	BotController->RunBehaviorTree(BotController->BehaviorTree);

	return Originals::OnBotSpawned(BotMutator, PlayerPawn);
}

void Bots::InitializeInventory(AFortAthenaAIBotController* BotController, UFortAthenaAIBotInventoryItems* OverrideStartupInventory)
{
	if (BotController->Inventory == NULL)
		BotController->Inventory = Utils::SpawnActor<AFortInventory>(FVector(), FRotator(), AFortInventory::StaticClass(), BotController);

	AddStartingItemsToInventory(BotController);

	return Originals::InitializeInventory(BotController, OverrideStartupInventory);
}

UFortWorldItem* CreateItem(AFortAthenaAIBotController* BotController, UFortItemDefinition* ItemDefinition, int32 Count)
{
	UFortWorldItem* WorldItem = Cast<UFortWorldItem>(ItemDefinition->CreateTemporaryItemInstanceBP(Count, 0));

	if (WorldItem == NULL)
		return NULL;

	WorldItem->OwnerInventory = BotController->Inventory;
	WorldItem->ItemEntry.ItemDefinition = ItemDefinition;
	WorldItem->ItemEntry.Count = Count;

	if (ItemDefinition->bInventorySizeLimited)
	{
		for (int32 SlotIndex = 0; SlotIndex < 6; ++SlotIndex)
		{
			FFortBotInventoryInfo& SlotItem = BotController->SlotItems[SlotIndex];

			if (SlotItem.FortItem == NULL || SlotItem.ItemDefinition == NULL)
			{
				SlotItem.FortItem = WorldItem;
				SlotItem.ItemDefinition = ItemDefinition;
				SlotItem.ItemType = ItemDefinition->ItemType;

				static EFortWeaponType(*FindWeaponType)(UFortItemDefinition*) = decltype(FindWeaponType)(InSDKUtils::GetImageBase() + 0x20E54E0);
				SlotItem.WeaponType = FindWeaponType(ItemDefinition);

				break;
			}
		}
	}

	BotController->Inventory->Inventory.ItemInstances.Add(WorldItem);
	BotController->Inventory->Inventory.ReplicatedEntries.Add(WorldItem->ItemEntry);
	BotController->Inventory->Inventory.MarkItemDirty(WorldItem->ItemEntry);

	BotController->Inventory->HandleInventoryLocalUpdate();

	return WorldItem;
}

// Not sure what the hook is, so i will call manually for now.

void Bots::AddStartingItemsToInventory(AFortAthenaAIBotController* BotController)
{
	if (BotController->StartupInventory == NULL)
	{
		BotController->StartupInventory = Utils::NewObject<UFortAthenaAIBotInventoryItems>(BotController);

		BotController->StartupInventory->Items.Add(BotController->PlayerBotPawn->CosmeticLoadout.Pickaxe->WeaponDefinition); // bit scuffed
	}

	if (BotController->StartupInventory)
	{
		for (UFortItemDefinition* Item : BotController->StartupInventory->Items)
		{
			CreateItem(BotController, Item, 1);
		}

		FFortItemEntry& ItemEntry = BotController->Inventory->Inventory.ReplicatedEntries[0];

		ItemEntry.ItemDefinition = BotController->PlayerBotPawn->CosmeticLoadout.Pickaxe->WeaponDefinition;

		BotController->PlayerBotPawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)ItemEntry.ItemDefinition, ItemEntry.ItemGuid);
	}
}

AFortAthenaAIBotController* Bots::GetSquadLeader(AFortAthenaAIBotController* BotController)
{
	return BotController;
}

void Bots::GetCosmeticLoadout(AFortAthenaAIBotController* BotController, void* a2, void* a3)
{
	Originals::GetCosmeticLoadout(BotController, a2, a3);

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(BotController->PlayerState);

	if (PlayerState == NULL)
		return;

	AFortPlayerPawnAthena* PlayerPawn = Cast<AFortPlayerPawnAthena>(BotController->PlayerBotPawn);

	if (PlayerState == NULL)
		return;

	static UDataTable* PheobeEmotes = Utils::StaticLoadObject<UDataTable>("/Game/Athena/AI/Phoebe/Cosmetic/Pheobe_Cosmetic_Emotes.Pheobe_Cosmetic_Emotes");
	static UDataTable* PhoebeCharacters = Utils::StaticLoadObject<UDataTable>("/Game/Athena/AI/Phoebe/Cosmetic/CosmeticLibraries/Basic/Phoebe_Cosmetic_Characters_Basic.Phoebe_Cosmetic_Characters_Basic");

	for (const auto& [RowName, RowValue] : PheobeEmotes->RowMap)
	{
		FFortBotCosmeticItemDataTableRow* TableRow = (FFortBotCosmeticItemDataTableRow*)RowValue;

		if (TableRow != NULL)
		{
			UAthenaDanceItemDefinition* Dance = (UAthenaDanceItemDefinition*)UKismetSystemLibrary::GetObjectFromPrimaryAssetId(TableRow->PrimaryAssetId);

			if (Dance)
			{
				BotController->CosmeticLoadoutBC.Dances.Add(Dance);
			}
		}
	}

	TArray<FFortBotCosmeticItemDataTableRow*> PhoebeCharactersRows;

	for (const auto& [RowName, RowValue] : PhoebeCharacters->RowMap)
	{
		FFortBotCosmeticItemDataTableRow* TableRow = (FFortBotCosmeticItemDataTableRow*)RowValue;

		if (TableRow != NULL)
			PhoebeCharactersRows.Add(TableRow);
	}

	int32 CharacterIndex = UKismetMathLibrary::RandomIntegerInRange(0, PhoebeCharactersRows.Num() - 1);
	FFortBotCosmeticItemDataTableRow* CharacterRow = PhoebeCharactersRows[CharacterIndex];

	if (UKismetMathLibrary::RandomBoolWithWeight(CharacterRow->Weight))
	{
		UAthenaCharacterItemDefinition* CharacterItemDefinition = (UAthenaCharacterItemDefinition*)UKismetSystemLibrary::GetObjectFromPrimaryAssetId(CharacterRow->PrimaryAssetId);

		if (CharacterItemDefinition && CharacterItemDefinition->HeroDefinition && CharacterItemDefinition->HeroDefinition->Specializations.IsValid())
		{
			for (int i = 0; i < CharacterItemDefinition->HeroDefinition->Specializations.Num(); i++)
			{
				UFortHeroSpecialization* Specialization = Utils::StaticLoadObject<UFortHeroSpecialization>(UKismetStringLibrary::Conv_NameToString(CharacterItemDefinition->HeroDefinition->Specializations[i].ObjectID.AssetPathName).ToString());

				if (Specialization)
				{
					for (int j = 0; j < Specialization->CharacterParts.Num(); j++)
					{
						UCustomCharacterPart* CustomCharacterPart = Utils::StaticLoadObject<UCustomCharacterPart>(UKismetStringLibrary::GetDefaultObj()->Conv_NameToString(Specialization->CharacterParts[j].ObjectID.AssetPathName).ToString());

						if (CustomCharacterPart)
							PlayerState->CharacterData.Parts[(uintptr_t)CustomCharacterPart->CharacterPartType] = CustomCharacterPart;
					}
				}
			}
		}

		PlayerPawn->CosmeticLoadout.Character = CharacterItemDefinition;
	}

	PlayerPawn->OnRep_CosmeticLoadout();

	//BotController->CachedBotManager->SpecialActorMinimapIconBrush.ResourceObject = PlayerPawn->CosmeticLoadout.Character->GetSmallPreviewImageBrush().ResourceObject;
}

void Bots::EnterAircraft(AFortAthenaAIBotController* BotController, AFortAthenaAircraft* InAircraft)
{
	BotController->BehaviorTree = Utils::StaticLoadObject<UBehaviorTree>("/Game/Athena/AI/Phoebe/BehaviorTrees/BT_Phoebe.BT_Phoebe");
	BotController->RunBehaviorTree(BotController->BehaviorTree);

	if (UFortControllerComponent_Aircraft* AircraftComponent = Cast<UFortControllerComponent_Aircraft>(BotController->GetComponentByClass(UFortControllerComponent_Aircraft::StaticClass())))
	{
		AircraftComponent->EnterAircraft(InAircraft);
	}

	if (UFortServerBotManagerAthena* BotManager = BotController->CachedBotManager)
	{
		const float MinDelay = UFortScalableFloatUtils::GetValueAtLevel(BotManager->ThankBusDriverMinTime, 0);
		const float MaxDelay = UFortScalableFloatUtils::GetValueAtLevel(BotManager->ThankBusDriverMaxTime, 0);

		const float Delay = UKismetMathLibrary::RandomIntegerInRange(MinDelay, MaxDelay);

		UKismetSystemLibrary::K2_SetTimer(BotController, TEXT("ThankBusDriver"), Delay, false, 0.0, 0.0f);
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

UFortServerBotManagerAthena* GetCachedServerManager()
{
	AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);

	return GameMode->ServerBotManager;
}

void Bots::Setup()
{
	Utils::Hook(InSDKUtils::GetImageBase() + 0x1654BB0, InitializeMMRInfos, (void**)&Originals::InitializeMMRInfos);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x165CB70, OnBotSpawned, (void**)&Originals::OnBotSpawned);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x12A7810, InitializeInventory, (void**)&Originals::InitializeInventory);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x1296DF0, GetCosmeticLoadout, (void**)&Originals::GetCosmeticLoadout);

	Utils::Hook(InSDKUtils::GetImageBase() + 0x12A46A0, GetSquadLeader);

	Utils::Rel32(InSDKUtils::GetImageBase() + 0x146E480, GetCachedServerManager);
}