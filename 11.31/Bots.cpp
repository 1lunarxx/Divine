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

	for (int i = 0; i < 6; i++)
	{
		for (UFortWorldItem* ItemInstance : BotController->Inventory->Inventory.ItemInstances)
		{
			if (ItemInstance == NULL)
				break;

			UFortItemDefinition* ItemDefinition = ItemInstance->GetItemDefinitionBP();

			if (ItemDefinition != NULL && ItemDefinition->bInventorySizeLimited)
			{
				FFortBotInventoryInfo& SlotItem = BotController->SlotItems[i];

				SlotItem.FortItem = ItemInstance;
				SlotItem.ItemDefinition = ItemDefinition;
				SlotItem.ItemType = ItemDefinition->ItemType;
				SlotItem.WeaponType = AFortWeapon::FindWeaponTypeForStats(ItemDefinition);
			}
		}
	}

	static UDataTable* PhoebeEmotes = Utils::StaticLoadObject<UDataTable>(TEXT("/Game/Athena/AI/Phoebe/Cosmetic/Pheobe_Cosmetic_Emotes.Pheobe_Cosmetic_Emotes"));

	if (PhoebeEmotes != NULL)
	{
		for (const auto& [RowName, RowValue] : PhoebeEmotes->RowMap)
		{
			if (RowValue == NULL)
				continue;

			FFortBotCosmeticItemDataTableRow* TableRow = (FFortBotCosmeticItemDataTableRow*)RowValue;

			UAthenaDanceItemDefinition* Dance = (UAthenaDanceItemDefinition*)UKismetSystemLibrary::GetObjectFromPrimaryAssetId(TableRow->PrimaryAssetId);

			if (Dance)
				BotController->CosmeticLoadoutBC.Dances.Add(Dance);
		}
	}

	BotController->Skill = 1000.0f;
}

void Bots::InitializeInventory(AFortAthenaAIBotController* BotController, UFortAthenaAIBotInventoryItems* OverrideStartupInventory)
{
	if (BotController->Inventory == NULL)
	{
		BotController->Inventory = Utils::SpawnActor<AFortInventory>(FVector(), FRotator(), AFortInventory::StaticClass(), BotController);
	}

	if (UFortItemDefinition* Item = BotController->StartupInventory->Items[UKismetMathLibrary::RandomIntegerInRange(1, BotController->StartupInventory->Items.Num() - 1)])
	{
		CreateItem(BotController, Item, 1);
		CreateItem(BotController, BotController->StartupInventory->Items[0], 1);

		if (UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(Item))
		{
			if (WeaponItemDefinition->GetAmmoWorldItemDefinition_BP() && WeaponItemDefinition->GetAmmoWorldItemDefinition_BP() != WeaponItemDefinition)
			{
				CreateItem(BotController, WeaponItemDefinition->GetAmmoWorldItemDefinition_BP(), 30);
			}
		}
	}

	return Originals::InitializeInventory(BotController, OverrideStartupInventory);
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
			if (Entry.ItemDefinition->IsA(UFortWeaponMeleeItemDefinition::StaticClass()))
				continue;

			Inventory::SpawnPickup(BotController->PlayerBotPawn->K2_GetActorLocation(), &Entry, Entry.Count, BotController->PlayerBotPawn, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::PlayerElimination);
		}
	}

	if (AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(BotController->PlayerState))
	{
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
						AFortPlayerStateAthena* TeamMember_PlayerState = Cast<AFortPlayerStateAthena>(TeamMember->PlayerState);

						if (TeamMember_PlayerState == NULL)
							continue;

						TeamMember_PlayerState->TeamKillScore++;
						TeamMember_PlayerState->OnRep_TeamKillScore();

						TeamMember_PlayerState->ClientReportTeamKill(TeamMember_PlayerState->TeamKillScore);
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

	NewItem->ItemEntry.ItemDefinition = ItemDefinition;
	NewItem->ItemEntry.Count = Count;

	if (UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition))
		NewItem->ItemEntry.LoadedAmmo = Inventory::GetStats(WeaponItemDefinition)->ClipSize;
	 
	if (ItemDefinition->bInventorySizeLimited)
	{
		for (int32 SlotIndex = 0; SlotIndex < 6; ++SlotIndex)
		{
			FFortBotInventoryInfo& SlotItem = BotController->SlotItems[SlotIndex];

			if (SlotItem.FortItem == NULL || SlotItem.ItemDefinition == NULL)
			{
				SlotItem.FortItem = NewItem;

				SlotItem.ItemDefinition = ItemDefinition;
				SlotItem.ItemType = ItemDefinition->ItemType;
				SlotItem.WeaponType = AFortWeapon::FindWeaponTypeForStats(ItemDefinition);

				break;
			}
		}
	}

	BotController->Inventory->Inventory.ItemInstances.Add(NewItem);
	BotController->Inventory->Inventory.ReplicatedEntries.Add(NewItem->ItemEntry);

	BotController->Inventory->Inventory.MarkItemDirty(NewItem->ItemEntry);
	BotController->Inventory->HandleInventoryLocalUpdate();

	return NewItem;
}

UFortServerBotManagerAthena* Bots::GetBotManager()
{
	AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);

	return GameMode->ServerBotManager;
}

UObject* Bots::GetSquadLeader(AFortAthenaAIBotController* BotController)
{
	return BotController;
}

void Bots::Setup()
{
	Utils::Hook(InSDKUtils::GetImageBase() + 0x165CB70, OnPawnSpawned, (void**)&Originals::OnPawnSpawned);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x12AB760, OnPossessedPawnDied, (void**)&Originals::OnPossessedPawnDied);

	Utils::Hook(InSDKUtils::GetImageBase() + 0x12A7810, InitializeInventory, (void**)&Originals::InitializeInventory);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x1654BB0, InitializeMMRInfos, (void**)&Originals::InitializeMMRInfos);

	Utils::Hook(InSDKUtils::GetImageBase() + 0x12A46A0, GetSquadLeader);

	Utils::Rel32(InSDKUtils::GetImageBase() + 0x146E480, GetBotManager);
}