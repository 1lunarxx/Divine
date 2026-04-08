#include "pch.h"
#include "Bots.h"
#include "Utils.h"
#include "Inventory.h"
#include "Configuration.h"

void Bots::InitializeMMRInfos(AFortAthenaMutator_Bots* BotMutator)
{
	*(uint8*)((__int64)(BotMutator) + 0x428) = UKismetMathLibrary::RandomIntegerInRange(0, 29); // CachedHumanPlayerCount

	return Originals::InitializeMMRInfos(BotMutator);
}

void Bots::OnBotSpawned(AFortAthenaMutator_Bots* BotMutator, AFortPlayerPawn* PlayerPawn)
{
	Originals::OnBotSpawned(BotMutator, PlayerPawn);

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

	BotController->BehaviorTree = Utils::StaticLoadObject<UBehaviorTree>(TEXT("/Game/Athena/AI/Phoebe/BehaviorTrees/BT_Phoebe_Warmup.BT_Phoebe_Warmup"));
	BotController->RunBehaviorTree(BotController->BehaviorTree);
}

void Bots::InitializeInventory(AFortAthenaAIBotController* BotController, UFortAthenaAIBotInventoryItems* OverrideStartupInventory)
{
	if (BotController->Inventory == NULL)
		BotController->Inventory = Utils::SpawnActor<AFortInventory>(FVector(), FRotator(), AFortInventory::StaticClass(), BotController);

	Bots::AddStartingItemsToInventory(BotController);

	return Originals::InitializeInventory(BotController, OverrideStartupInventory);
}

void Bots::AddStartingItemsToInventory(AFortAthenaAIBotController* BotController)
{
	for (UFortItemDefinition* Item : BotController->StartupInventory->Items)
	{
		CreateItem(BotController, Item, 1);
	}

	for (const FFortItemEntry& ItemEntry : BotController->Inventory->Inventory.ReplicatedEntries)
	{
		if (ItemEntry.ItemDefinition->IsA(UFortWeaponMeleeItemDefinition::StaticClass()))
		{
			BotController->PlayerBotPawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)ItemEntry.ItemDefinition, ItemEntry.ItemGuid);

			break;
		}
	}
}

// Will figure it out later

void Bots::InitializeCosmeticLoadout(AFortAthenaAIBotController* BotController, void* a2, void* a3)
{
	Originals::InitializeCosmeticLoadout(BotController, a2, a3);

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(BotController->PlayerState);

	if (PlayerState == NULL)
		return;

	AFortPlayerPawnAthena* PlayerPawn = Cast<AFortPlayerPawnAthena>(BotController->PlayerBotPawn);

	if (PlayerState == NULL)
		return;

	static UDataTable* PhoebeCharacters = Utils::StaticLoadObject<UDataTable>(TEXT("/Game/Athena/AI/Phoebe/Cosmetic/CosmeticLibraries/Basic/Phoebe_Cosmetic_Characters_Basic.Phoebe_Cosmetic_Characters_Basic"));
	static UDataTable* PheobeEmotes = Utils::StaticLoadObject<UDataTable>(TEXT("/Game/Athena/AI/Phoebe/Cosmetic/Pheobe_Cosmetic_Emotes.Pheobe_Cosmetic_Emotes"));

	TArray<UAthenaCharacterItemDefinition*> PhoebeCharacterItemDefinitions;

	for (const auto& [RowName, RowValue] : PheobeEmotes->RowMap)
	{
		FFortBotCosmeticItemDataTableRow* TableRow = (FFortBotCosmeticItemDataTableRow*)RowValue;

		if (TableRow != NULL)
		{
			UAthenaDanceItemDefinition* Dance = (UAthenaDanceItemDefinition*)UKismetSystemLibrary::GetObjectFromPrimaryAssetId(TableRow->PrimaryAssetId);

			if (Dance)
				PlayerPawn->CosmeticLoadout.Dances.Add(Dance);
		}
	}

	for (const auto& [RowName, RowValue] : PhoebeCharacters->RowMap)
	{
		FFortBotCosmeticItemDataTableRow* TableRow = (FFortBotCosmeticItemDataTableRow*)RowValue;

		if (TableRow != NULL)
		{
			UAthenaCharacterItemDefinition* Character = (UAthenaCharacterItemDefinition*)UKismetSystemLibrary::GetObjectFromPrimaryAssetId(TableRow->PrimaryAssetId);

			if (Character)
				PhoebeCharacterItemDefinitions.Add(Character);
		}
	}

	int32 SelectedPhoebeCharacter = UKismetMathLibrary::RandomIntegerInRange(0, PhoebeCharacterItemDefinitions.Num() - 1);

	UAthenaCharacterItemDefinition* PhoebeCharacter = PhoebeCharacterItemDefinitions[SelectedPhoebeCharacter];

	if (PhoebeCharacter != NULL)
	{
		if (PhoebeCharacter->HeroDefinition && PhoebeCharacter->HeroDefinition->Specializations.IsValid())
		{
			for (TSoftObjectPtr<UFortHeroSpecialization>& HeroSpecialization : PhoebeCharacter->HeroDefinition->Specializations)
			{
				UFortHeroSpecialization* Specialization = Utils::StaticLoadObject<UFortHeroSpecialization>(UKismetStringLibrary::Conv_NameToString(HeroSpecialization.ObjectID.AssetPathName).CStr());

				if (Specialization)
				{
					for (TSoftObjectPtr<UCustomCharacterPart>& CharacterPart : Specialization->CharacterParts)
					{
						UCustomCharacterPart* CustomCharacterPart = Utils::StaticLoadObject<UCustomCharacterPart>(UKismetStringLibrary::Conv_NameToString(CharacterPart.ObjectID.AssetPathName).CStr());

						if (CustomCharacterPart)
							PlayerState->CharacterData.Parts[(uintptr_t)CustomCharacterPart->CharacterPartType] = CustomCharacterPart;
					}
				}
			}
		}

		PlayerPawn->CosmeticLoadout.Character = PhoebeCharacter;
	}

	PlayerPawn->OnRep_CosmeticLoadout();

	if (Configuration::bUseCustomMinimap)
	{
		BotController->CachedBotManager->SpecialActorMinimapIconBrush.ImageSize = FVector2D(50, 50);
		BotController->CachedBotManager->SpecialActorMinimapIconBrush.ResourceObject = PlayerPawn->CosmeticLoadout.Character->GetSmallPreviewImageBrush().ResourceObject;
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
	BotController->BehaviorTree = Utils::StaticLoadObject<UBehaviorTree>(TEXT("/Game/Athena/AI/Phoebe/BehaviorTrees/BT_Phoebe.BT_Phoebe"));
	BotController->RunBehaviorTree(BotController->BehaviorTree);

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
	UFortWorldItem* WorldItem = Cast<UFortWorldItem>(ItemDefinition->CreateTemporaryItemInstanceBP(Count, 0));

	if (WorldItem == NULL)
		return NULL;

	WorldItem->OwnerInventory = BotController->Inventory;

	WorldItem->ItemEntry.ItemDefinition = ItemDefinition;
	WorldItem->ItemEntry.Count = Count;

	if (UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition))
	{
		WorldItem->ItemEntry.LoadedAmmo = Inventory::GetStats(WeaponItemDefinition)->ClipSize;
	}

	if (ItemDefinition->bInventorySizeLimited)
	{
		for (int32 SlotIndex = 0; SlotIndex < 6; SlotIndex++)
		{
			FFortBotInventoryInfo& SlotItem = BotController->SlotItems[SlotIndex];

			if (SlotItem.FortItem == NULL || SlotItem.ItemDefinition == NULL)
			{
				SlotItem.FortItem = WorldItem;
				SlotItem.ItemDefinition = ItemDefinition;
				SlotItem.ItemType = ItemDefinition->ItemType;
				SlotItem.WeaponType = AFortWeapon::FindWeaponTypeForStats(ItemDefinition);

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
	Utils::Hook(InSDKUtils::GetImageBase() + 0x1654BB0, InitializeMMRInfos, (void**)&Originals::InitializeMMRInfos);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x165CB70, OnBotSpawned, (void**)&Originals::OnBotSpawned);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x12A7810, InitializeInventory, (void**)&Originals::InitializeInventory);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x12AB760, OnPossessedPawnDied, (void**)&Originals::OnPossessedPawnDied);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x1296DF0, InitializeCosmeticLoadout, (void**)&Originals::InitializeCosmeticLoadout);

	Utils::Hook(InSDKUtils::GetImageBase() + 0x12A46A0, GetSquadLeader);

	Utils::Rel32(InSDKUtils::GetImageBase() + 0x146E480, GetServerBotManager);
}