#include "pch.h"
#include "Player.h"
#include "Utils.h"
#include "Inventory.h"

void Player::ServerAcknowledgePossession(AFortPlayerControllerAthena* PlayerController, APawn* P)
{
	PlayerController->AcknowledgedPawn = P;

    AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

    UFortKismetLibrary::UpdatePlayerCustomCharacterPartsVisualization(PlayerState);

    PlayerState->OnRep_CharacterData();

	UFortGlobals::InitializePlayerGameplayAbilities(Utils::GetInterfaceAddress<IAbilitySystemInterface>(PlayerState));

	if (AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode))
	{
		for (const FItemAndCount& StartingItem : GameMode->StartingItems)
		{
			if (StartingItem.Count > 0)
			{
				Inventory::CreateItem(PlayerController->WorldInventory, StartingItem.Item, StartingItem.Count);
			}
		}

		UFortItemDefinition* WoodItemData = Utils::StaticFindObject<UFortItemDefinition>(TEXT("WoodItemData"), ANY_PACKAGE);

		Inventory::CreateItem(PlayerController->WorldInventory, WoodItemData, 50);
		Inventory::CreateItem(PlayerController->WorldInventory, PlayerController->CosmeticLoadoutPC.Pickaxe->WeaponDefinition);
	}
}

void Player::ServerPlayEmoteItem(AFortPlayerControllerAthena* PlayerController, UFortItemDefinition* EmoteAsset, float EmoteRandomNumber)
{
	if (EmoteAsset == NULL)
		return;

	AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;

	if (PlayerPawn == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	static UClass* AbilityClass = NULL;

	if (UAthenaDanceItemDefinition* DanceItemDefinition = Cast< UAthenaDanceItemDefinition>(EmoteAsset))
	{
		AbilityClass = DanceItemDefinition->CustomDanceAbility.Get() ? DanceItemDefinition->CustomDanceAbility.Get() : Utils::StaticFindObject<UClass>(TEXT("/Game/Abilities/Emotes/GAB_Emote_Generic.GAB_Emote_Generic_C"));

		PlayerPawn->bMovingEmote = DanceItemDefinition->bMovingEmote;
		PlayerPawn->EmoteWalkSpeed = DanceItemDefinition->WalkForwardSpeed;
		PlayerPawn->bMovingEmoteForwardOnly = DanceItemDefinition->bMoveForwardOnly;
	}
	else if (UAthenaSprayItemDefinition* SprayItemDefinition = Cast<UAthenaSprayItemDefinition>(EmoteAsset))
	{
		AbilityClass = Utils::StaticFindObject<UClass>(TEXT("/Game/Abilities/Sprays/GAB_Spray_Generic.Default__GAB_Spray_Generic_C"));
	}
	else if (UAthenaToyItemDefinition* ToyItemDefinition = Cast<UAthenaToyItemDefinition>(EmoteAsset))
	{
		if (ToyItemDefinition->ToySpawnAbility.Get())
			AbilityClass = ToyItemDefinition->ToySpawnAbility.Get()->Class;
	}

	FGameplayAbilitySpec Spec = FGameplayAbilitySpec();

	Spec.ConstructAbilitySpec(Cast<UGameplayAbility>(AbilityClass->ClassDefaultObject), 1, -1, EmoteAsset);

	PlayerState->AbilitySystemComponent->GiveAbilityAndActivateOnce(&Spec.Handle, &Spec);
}

void Player::GetPlayerViewPoint(AFortPlayerControllerAthena* PlayerController, FVector& Location, FRotator& Rotation)
{
	if (PlayerController->GetViewTarget())
	{
		Location = PlayerController->GetViewTarget()->K2_GetActorLocation();
		Rotation = PlayerController->GetControlRotation();
	}
	else
	{
		return Originals::GetPlayerViewPoint(PlayerController, Location, Rotation);
	}
}

void Player::ServerSendZiplineState(AFortPlayerPawn* PlayerPawn, FZiplinePawnState& InZiplineState)
{
	PlayerPawn->ZiplineState = InZiplineState;
	PlayerPawn->OnRep_ZiplineState();

	float Velocity = 0.f;

	if (InZiplineState.bJumped)
		PlayerPawn->LaunchCharacterJump({ Velocity >= -750 ? UKismetMathLibrary::FMin(Velocity, 750) : -750, Velocity >= -750 ? UKismetMathLibrary::FMin(Velocity, 750) : -750, 1500 }, false, false, true, true);
}

void Player::RemoveAllInventoryItems(AFortPlayerControllerAthena* PlayerController)
{
	PlayerController->WorldInventory->Inventory.ReplicatedEntries.Free();
	PlayerController->WorldInventory->Inventory.ItemInstances.Free();

	PlayerController->WorldInventory->Inventory.MarkArrayDirty();
}

void Player::ServerExecuteInventoryItem(AFortPlayerController* PlayerController, FGuid& ItemGuid)
{
	AFortPlayerPawnAthena* PlayerPawn = Cast<AFortPlayerPawnAthena>(PlayerController->MyFortPawn);

	if (PlayerPawn == NULL)
		return;

	FFortItemEntry* ItemEntry = Inventory::GetItemEntry(PlayerController->WorldInventory, ItemGuid);

	if (ItemEntry == NULL)
		return;

	UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemEntry->ItemDefinition);

	if (WeaponItemDefinition == NULL)
		return;

	PlayerPawn->EquipWeaponDefinition(WeaponItemDefinition, ItemGuid);
}

void Player::ServerAttemptAircraftJump(UFortControllerComponent_Aircraft* Component_Aircraft, FRotator& ClientRotation)
{
	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(Component_Aircraft->GetOwner());

	if (PlayerController == NULL)
		return;

	AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);

	if (GameMode == NULL)
		return;

	GameMode->RestartPlayer(PlayerController);

	PlayerController->ClientSetRotation(ClientRotation, false);
}

void Player::MovingEmoteStopped(AFortPawn* Pawn)
{
	Pawn->bMovingEmote = false;

	return Originals::MovingEmoteStopped(Pawn);
}

void Player::ServerBeginEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* Building)
{
	if (PlayerController == NULL || PlayerController->MyFortPawn == NULL || Building == NULL)
		return;

	AFortPlayerPawnAthena* Pawn = Cast<AFortPlayerPawnAthena>(PlayerController->Pawn);

	if (Pawn == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	FFortItemEntry* ItemEntry = Inventory::GetItemEntry(PlayerController->WorldInventory, UFortEditToolItemDefinition::StaticClass());

	if (ItemEntry == NULL)
		return;

	UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemEntry->ItemDefinition);

	if (WeaponItemDefinition == NULL)
		return;

	AFortWeap_EditingTool* EditingTool = Cast<AFortWeap_EditingTool>(Pawn->EquipWeaponDefinition(WeaponItemDefinition, ItemEntry->ItemGuid));

	if (EditingTool == NULL)
		return;

	Building->SetEditingPlayer(PlayerState);

	EditingTool->EditActor = Building;
	EditingTool->OnRep_EditActor();
}

void Player::ServerEditBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* Building, TSubclassOf<ABuildingSMActor> NewClass, uint8 RotationIterations, bool bMirrored)
{
	if (Building == NULL || Building->EditingPlayer != PlayerController->PlayerState || Building->bDestroyed)
		return;

	Building->SetEditingPlayer(NULL);

	int32 CurrentBuildingLevel = Building->CurrentBuildingLevel;

	Building->ReplaceBuildingActor(EBuildingReplacementType::BRT_Edited, NewClass, CurrentBuildingLevel, RotationIterations, bMirrored, PlayerController);
}

void Player::ServerEndEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* Building)
{
	if (Building == NULL || Building->EditingPlayer != PlayerController->PlayerState || Building->bDestroyed)
		return;

	AFortPlayerPawnAthena* Pawn = Cast<AFortPlayerPawnAthena>(PlayerController->Pawn);

	if (Pawn == NULL)
		return;

	FFortItemEntry* ItemEntry = Inventory::GetItemEntry(PlayerController->WorldInventory, UFortEditToolItemDefinition::StaticClass());

	if (ItemEntry == NULL)
		return;

	UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemEntry->ItemDefinition);

	if (WeaponItemDefinition == NULL)
		return;

	AFortWeap_EditingTool* EditingTool = Cast<AFortWeap_EditingTool>(Pawn->EquipWeaponDefinition(WeaponItemDefinition, ItemEntry->ItemGuid));

	if (EditingTool == NULL)
		return;

	Building->SetEditingPlayer(NULL);

	EditingTool->EditActor = NULL;
	EditingTool->OnRep_EditActor();
}

void Player::ServerCreateBuildingActor(AFortPlayerController* PlayerController, FCreateBuildingActorData BuildingActorData)
{
	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);

	if (GameState == NULL)
		return;

	UClass* BuildingClass = ((AFortPlayerControllerAthena*)PlayerController)->BroadcastRemoteClientInfo->RemoteBuildableClass.Get();

	if (BuildingClass == NULL)
		return;

	TArray<ABuildingActor*> ExistingBuildings;
	EFortBuildPreviewMarkerOptionalAdjustment MarkerOptionalAdjustment;

	EFortStructuralGridQueryResults QueryResults = GameState->StructuralSupportSystem->CanAddBuildingActorClassToGrid(UWorld::GetWorld(), BuildingClass, BuildingActorData.BuildLoc, BuildingActorData.BuildRot, BuildingActorData.bMirrored, &ExistingBuildings, &MarkerOptionalAdjustment, false);

	if (QueryResults == EFortStructuralGridQueryResults::CanAdd)
	{
		for (ABuildingActor* ExistingBuilding : ExistingBuildings)
			ExistingBuilding->K2_DestroyActor();

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

void Player::ServerReviveFromDBNO(AFortPlayerPawnAthena* PlayerPawn, AController* EventInstigator)
{
	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(PlayerPawn->Controller);

	if (PlayerController == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	for (FGameplayAbilitySpec& AbilitySpec : PlayerState->AbilitySystemComponent->ActivatableAbilities.Items)
	{
		if (AbilitySpec.Ability->IsA(Utils::StaticLoadObject<UClass>(TEXT("/Game/Abilities/NPC/Generic/GAB_AthenaDBNO.GAB_AthenaDBNO_C"))))
		{
			PlayerState->AbilitySystemComponent->ServerCancelAbility(AbilitySpec.Handle, AbilitySpec.ActivationInfo);
			PlayerState->AbilitySystemComponent->ServerEndAbility(AbilitySpec.Handle, AbilitySpec.ActivationInfo, AbilitySpec.ActivationInfo.PredictionKeyWhenActivated);
			PlayerState->AbilitySystemComponent->ClientCancelAbility(AbilitySpec.Handle, AbilitySpec.ActivationInfo);
			PlayerState->AbilitySystemComponent->ClientEndAbility(AbilitySpec.Handle, AbilitySpec.ActivationInfo);

			break;
		}
	}

	PlayerPawn->bIsDBNO = false;
	PlayerPawn->OnRep_IsDBNO();

	PlayerPawn->SetHealth(30);

	PlayerController->RespawnPlayerAfterDeath(false);
	PlayerController->ClientOnPawnRevived(EventInstigator);
}

void Player::ServerAddMapMarker(UAthenaMarkerComponent* MarkerComponent, FFortClientMarkerRequest& MarkerRequest)
{
	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(MarkerComponent->GetOwner());

	if (PlayerController == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	FFortWorldMarkerData WorldMarkerData = FFortWorldMarkerData();

	WorldMarkerData.ConstructWorldMarkerData();

	FMarkerID MarkerID{};

	MarkerID.PlayerID = PlayerState->PlayerID;
	MarkerID.InstanceID = MarkerRequest.InstanceID;

	WorldMarkerData.MarkerID = MarkerID;
	WorldMarkerData.Owner = PlayerState;

	WorldMarkerData.MarkerType = MarkerRequest.MarkerType;
	WorldMarkerData.WorldPosition = MarkerRequest.WorldPosition;
	WorldMarkerData.WorldPositionOffset = MarkerRequest.WorldPositionOffset;
	WorldMarkerData.WorldNormal = MarkerRequest.WorldNormal;

	WorldMarkerData.bHasCustomDisplayInfo = false;

	for (auto& _TeamMember : PlayerState->PlayerTeam->TeamMembers)
	{
		AFortPlayerControllerAthena* TeamMember = Cast<AFortPlayerControllerAthena>(_TeamMember);

		if (TeamMember == NULL || TeamMember == PlayerController)
			continue;

		UAthenaMarkerComponent* TeamMemberMarkerComponent = TeamMember->MarkerComponent;

		if (TeamMemberMarkerComponent == NULL)
			continue;

		TeamMemberMarkerComponent->ClientAddMarker(WorldMarkerData);
	}
}

void Player::ServerRemoveMapMarker(UAthenaMarkerComponent* MarkerComponent, FMarkerID MarkerID, ECancelMarkerReason CancelReason)
{
	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(MarkerComponent->GetOwner());

	if (PlayerController == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	for (auto& _TeamMember : PlayerState->PlayerTeam->TeamMembers)
	{
		AFortPlayerControllerAthena* TeamMember = Cast<AFortPlayerControllerAthena>(_TeamMember);

		if (TeamMember == NULL || TeamMember == PlayerController)
			continue;

		UAthenaMarkerComponent* TeamMemberMarkerComponent = TeamMember->MarkerComponent;

		if (TeamMemberMarkerComponent == NULL)
			continue;

		TeamMemberMarkerComponent->ClientCancelMarker(MarkerID);
	}
}

void Player::ServerCheat(AFortPlayerController* PlayerController, FString& Msg)
{
	AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);

	if (GameMode == NULL)
		return;

	std::string Message = Msg.ToString();

	if (Message == "god")
	{
		if (PlayerController->MyFortPawn == NULL)
			return;

		PlayerController->MyFortPawn->bCanBeDamaged = false;

		PlayerController->MyFortPawn->HealthSet->Health.Minimum = 100;
		PlayerController->MyFortPawn->HealthSet->OnRep_Health();

		PlayerController->ClientMessage(L"God mode enabled!", FName(), 1);
	}

	if (Message == "randombot")
	{
		if (AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState))
		{
			if (GameState->GamePhase > EAthenaGamePhase::Aircraft)
			{
				if (GameMode->ServerBotManager->PlayerBots.Num() > 0)
				{
					int32 Index = UKismetMathLibrary::RandomIntegerInRange(0, GameMode->ServerBotManager->PlayerBots.Num() - 1);

					if (AFortPlayerPawnAthena* PlayerBotPawn = GameMode->ServerBotManager->PlayerBots[Index].BotController->PlayerBotPawn)
					{
						PlayerController->Pawn->K2_TeleportTo(PlayerBotPawn->K2_GetActorLocation(), {});
					}
				}
			}
			else
			{
				PlayerController->ClientMessage(L"Invalid Gamephase! Please wait til the aircraft has exited.", FName(), 1);
			}
		}
	}

	if (Message.contains("sethealth"))
	{
		if (Message.rfind("sethealth", 0) == 0)
		{
			std::string sethealth = "sethealth";
			std::string args;

			if (Message.size() > sethealth.size() + 1)
				args = Message.substr(sethealth.size() + 1);

			std::stringstream ss(args);

			float Health;
			ss >> Health;

			AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;

			if (PlayerPawn == NULL)
				return;

			if (Health < 1)
				PlayerController->ClientMessage(L"Couldn't Set Health Lower Then 1", FName(), 1);

			PlayerPawn->SetHealth(Health);

			PlayerController->ClientMessage(L"Health Changed!", FName(), 1);
		}
	}

	if (Message.contains("setshield"))
	{
		if (Message.rfind("setshield", 0) == 0)
		{
			std::string setshield = "setshield";
			std::string args;

			if (Message.size() > setshield.size() + 1)
				args = Message.substr(setshield.size() + 1);

			std::stringstream ss(args);

			float Shield;
			ss >> Shield;

			AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;

			if (PlayerPawn == NULL)
				return;

			PlayerPawn->SetShield(Shield);

			PlayerController->ClientMessage(L"Shield Changed!", FName(), 1);
		}
	}

	if (Message.contains("launch"))
	{
		if (Message.rfind("launch", 0) == 0)
		{
			std::string launch = "launch";
			std::string args;

			if (Message.size() > launch.size() + 1)
				args = Message.substr(launch.size() + 1);

			std::stringstream ss(args);

			float X{}, Y{}, Z{};
			ss >> X >> Y >> Z;

			AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;

			if (PlayerPawn == NULL)
				return;

			PlayerPawn->LaunchCharacter(FVector(X, Y, Z), false, false);

			PlayerController->ClientMessage(L"Launched Character!", FName(), 1);
		}
	}

	if (Message.rfind("giveitem", 0) == 0)
	{
		std::string giveitem = "giveitem";
		std::string args;

		if (Message.size() > giveitem.size() + 1)
			args = Message.substr(giveitem.size() + 1);

		std::stringstream ss(args);

		std::string WeaponId;

		int Count { 1 };
		int LoadedAmmo { 2 };

		ss >> WeaponId;

		if (!(ss >> Count))
			Count = 1;

		if (!(ss >> LoadedAmmo))
			LoadedAmmo = 2;

		std::wstring wid(WeaponId.begin(), WeaponId.end());

		auto ItemDefinition = Cast<UFortWorldItemDefinition>(Utils::StaticFindObject(wid.c_str(), ANY_PACKAGE));

		if (ItemDefinition == NULL)
		{
			PlayerController->ClientMessage(L"Failed to find item definition! Are you entering the correct WID?", FName(), 1);
			return;
		}

		Inventory::CreateItem(PlayerController->WorldInventory, ItemDefinition, Count, LoadedAmmo);

		PlayerController->ClientMessage(L"Gave item!", FName(), 1);
	}
}

void Player::ServerAttemptInventoryDrop(AFortPlayerController* PlayerController, FGuid& ItemGuid, int32 Count, bool bTrash)
{
	AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;

	if (PlayerPawn == NULL)
		return;

	FFortItemEntry* ItemEntry = Inventory::GetItemEntry(PlayerController->WorldInventory, ItemGuid);

	if (ItemEntry == NULL || ItemEntry->Count - Count < 0)
		return;

	UFortWorldItem* WorldItem = Inventory::GetWorldItem(PlayerController->WorldInventory, ItemGuid);

	if (WorldItem == NULL)
		return;

	ItemEntry->Count -= Count;

	Inventory::SpawnPickup(PlayerPawn->K2_GetActorLocation() + PlayerPawn->GetActorForwardVector() * 70.f + FVector(0, 0, 50), ItemEntry, Count, PlayerPawn, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset);

	if (ItemEntry->Count <= 0 || Count < 0)
	{
		Inventory::RemoveItem(PlayerController->WorldInventory, ItemGuid);
	}
	else
	{
		WorldItem->ItemEntry.Count = ItemEntry->Count;

		Inventory::UpdateItemEntry(PlayerController->WorldInventory, ItemEntry);
	}
}

void Player::ServerHandlePickup(AFortPlayerPawn* PlayerPawn, AFortPickup* Pickup, float InFlyTime, FVector& InStartDirection, bool bPlayPickupSound)
{
	if (Pickup == NULL)
		return;

	Pickup->SetPickupTarget(PlayerPawn, InFlyTime / PlayerPawn->PickupSpeedMultiplier, InStartDirection);
}

void Player::GivePickupTo(AFortPickup* Pickup, IFortInventoryOwnerInterface* InventoryOwner, bool DestroyAfterPickup)
{
	Originals::GivePickupTo(Pickup, InventoryOwner, DestroyAfterPickup);

	AFortPlayerPawnAthena* PlayerPawn = Cast<AFortPlayerPawnAthena>(Pickup->PickupLocationData.PickupTarget);

	if (PlayerPawn == NULL)
		return;

	if (AFortPlayerController* PlayerController = Cast<AFortPlayerController>(PlayerPawn->Controller))
	{
		Inventory::CreateItem(PlayerController->WorldInventory, Pickup->PrimaryPickupItemEntry.ItemDefinition, Pickup->PrimaryPickupItemEntry.Count, Pickup->PrimaryPickupItemEntry.LoadedAmmo);
	}
}

void Player::Setup()
{
    Utils::Virtual<AFortPlayerControllerAthena>(0x10A, ServerAcknowledgePossession);
	Utils::Virtual<AFortPlayerControllerAthena>(0x209, ServerExecuteInventoryItem);
	Utils::Virtual<AFortPlayerControllerAthena>(0x1C3, ServerPlayEmoteItem);

	Utils::Virtual<AFortPlayerControllerAthena>(0xE08 / 8, ServerCheat);
	Utils::Virtual<AFortPlayerControllerAthena>(0x10E8 / 8, ServerAttemptInventoryDrop);

	Utils::Virtual<AFortPlayerControllerAthena>(0x11B8 / 8, ServerBeginEditingBuildingActor);
	Utils::Virtual<AFortPlayerControllerAthena>(0x1190 / 8, ServerEditBuildingActor);
	Utils::Virtual<AFortPlayerControllerAthena>(0x11A8 / 8, ServerEndEditingBuildingActor);
	Utils::Virtual<AFortPlayerControllerAthena>(0x1180 / 8, ServerCreateBuildingActor);

	Utils::Virtual<AFortPickupAthena>(0xC7, GivePickupTo, (void**)&Originals::GivePickupTo);

	Utils::Virtual<AFortPlayerPawnAthena>(0x1E1, ServerSendZiplineState);
	Utils::Virtual<AFortPlayerPawnAthena>(0x1BF, ServerReviveFromDBNO);
	Utils::Virtual<AFortPlayerPawnAthena>(0xEB0 / 8, ServerHandlePickup);

	Utils::Virtual<UAthenaMarkerComponent>(0x7F, ServerAddMapMarker);
	Utils::Virtual<UAthenaMarkerComponent>(0x7D, ServerRemoveMapMarker);

	Utils::Virtual<UFortControllerComponent_Aircraft>(0x82, ServerAttemptAircraftJump);

	Utils::Hook(InSDKUtils::GetImageBase() + 0xBE8270, MovingEmoteStopped, (void**)&Originals::MovingEmoteStopped);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x1E181D0, GetPlayerViewPoint, (void**)&Originals::GetPlayerViewPoint);

	Utils::Rel32(InSDKUtils::GetImageBase() + 0x154E9E8, RemoveAllInventoryItems);
}