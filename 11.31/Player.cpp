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
				Inventory::GiveItem(PlayerController->WorldInventory, StartingItem.Item, StartingItem.Count);
			}
		}

		Inventory::GiveItem(PlayerController->WorldInventory, PlayerController->CosmeticLoadoutPC.Pickaxe->WeaponDefinition);
	}
}

void Player::ServerPlayEmoteItem(AFortPlayerControllerAthena* PlayerController, UFortItemDefinition* EmoteAsset, float EmoteRandomNumber)
{
	AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;

	if (PlayerPawn == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	UClass* AbilityClass = NULL;

	if (UAthenaDanceItemDefinition* DanceItemDefinition = Cast<UAthenaDanceItemDefinition>(EmoteAsset))
	{
		AbilityClass = DanceItemDefinition->CustomDanceAbility.Get() ? DanceItemDefinition->CustomDanceAbility.Get() : Utils::StaticFindObject<UClass>(TEXT("/Game/Abilities/Emotes/GAB_Emote_Generic.GAB_Emote_Generic_C"));

		PlayerPawn->bMovingEmote = DanceItemDefinition->bMovingEmote;
		PlayerPawn->EmoteWalkSpeed = DanceItemDefinition->WalkForwardSpeed;
		PlayerPawn->bMovingEmoteForwardOnly = DanceItemDefinition->bMoveForwardOnly;
	}

	if (AbilityClass != NULL)
	{
		FGameplayAbilitySpec Spec;
		Spec.ConstructAbilitySpec(Cast<UGameplayAbility>(AbilityClass->ClassDefaultObject), 1, -1, EmoteAsset);

		PlayerState->AbilitySystemComponent->GiveAbilityAndActivateOnce(&Spec.Handle, &Spec);
	}
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
		Originals::GetPlayerViewPoint(PlayerController, Location, Rotation);
	}
}

void Player::ServerSendZiplineState(AFortPlayerPawn* PlayerPawn, FZiplinePawnState& InZiplineState)
{
	PlayerPawn->ZiplineState = InZiplineState;
	PlayerPawn->OnRep_ZiplineState();

	if (InZiplineState.bJumped)
	{
		FVector Velocity;

		Velocity.X = 0;
		Velocity.Z = 1500;

		PlayerPawn->LaunchCharacterJump(Velocity, false, false, true, true);
	}
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

	if (UFortContextTrapItemDefinition* ContextTrapItemDefinition = Cast<UFortContextTrapItemDefinition>(WeaponItemDefinition))
	{
		PlayerPawn->PickUpActor(NULL, ContextTrapItemDefinition);
		PlayerPawn->CurrentWeapon->ItemEntryGuid = ItemGuid;

		if (AFortDecoTool_ContextTrap* ContextTrap = Cast<AFortDecoTool_ContextTrap>(PlayerPawn->CurrentWeapon))
		{
			ContextTrap->ContextTrapItemDefinition = ContextTrapItemDefinition;
		}
	}
	else
	{
		PlayerPawn->EquipWeaponDefinition(WeaponItemDefinition, ItemGuid);
	}
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

void Player::ServerAddMapMarker(UAthenaMarkerComponent* MarkerComponent, FFortClientMarkerRequest& MarkerRequest)
{
	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(MarkerComponent->GetOwner());

	if (PlayerController == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	FFortWorldMarkerData WorldMarkerData = FFortWorldMarkerData{};

	WorldMarkerData.CreateWorldMarkerData();

	FMarkerID MarkerID = FMarkerID{};

	MarkerID.InstanceID = MarkerRequest.InstanceID;
	MarkerID.PlayerID = WorldMarkerData.Owner->PlayerID;

	WorldMarkerData.Owner = PlayerState;
	WorldMarkerData.MarkerID = MarkerID;

	WorldMarkerData.MarkerType = MarkerRequest.MarkerType;
	WorldMarkerData.WorldPosition = MarkerRequest.WorldPosition;
	WorldMarkerData.WorldPositionOffset = MarkerRequest.WorldPositionOffset;
	WorldMarkerData.WorldNormal = MarkerRequest.WorldNormal;

	WorldMarkerData.bHasCustomDisplayInfo = false;

	if (AFortPickup* Pickup = Cast<AFortPickup>(MarkerRequest.MarkedActor))
	{
		WorldMarkerData.ItemDefinition = Pickup->PrimaryPickupItemEntry.ItemDefinition;
		WorldMarkerData.ItemCount = Pickup->PrimaryPickupItemEntry.Count;
	}

	for (AController* TeamMember : PlayerState->PlayerTeam->TeamMembers)
	{
		if (TeamMember == PlayerController)
			continue;

		if (AFortPlayerControllerAthena* TeamMember_Athena = Cast<AFortPlayerControllerAthena>(TeamMember))
		{
			UAthenaMarkerComponent* TeamMemberMarkerComponent = TeamMember_Athena->MarkerComponent;

			if (TeamMemberMarkerComponent != NULL)
				TeamMemberMarkerComponent->ClientAddMarker(WorldMarkerData);
		}
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

	for (AController* TeamMember : PlayerState->PlayerTeam->TeamMembers)
	{
		if (TeamMember == PlayerController)
			continue;

		if (AFortPlayerControllerAthena* TeamMember_Athena = Cast<AFortPlayerControllerAthena>(TeamMember))
		{
			UAthenaMarkerComponent* TeamMemberMarkerComponent = TeamMember_Athena->MarkerComponent;

			if (TeamMemberMarkerComponent != NULL)
				TeamMemberMarkerComponent->ClientCancelMarker(MarkerID);
		}
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

	if (Message == "disablegod")
	{
		if (PlayerController->MyFortPawn == NULL)
			return;

		PlayerController->MyFortPawn->bCanBeDamaged = true;

		PlayerController->MyFortPawn->HealthSet->Health.Minimum = 0;
		PlayerController->MyFortPawn->HealthSet->OnRep_Health();

		PlayerController->ClientMessage(L"God mode disabled!", FName(), 1);
	}

	if (Message.contains("demospeed"))
	{
		if (Message.rfind("demospeed", 0) == 0)
		{
			std::string Demospeed_ = "demospeed";
			std::string Args;

			if (Message.size() > Demospeed_.size() + 1)
				Args = Message.substr(Demospeed_.size() + 1);

			std::stringstream ss(Args);

			float Speed;
			ss >> Speed;

			std::wstring Command = L"demospeed " + std::to_wstring(Speed);

			FString Run(Command.c_str());
			UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), Run, nullptr);

			PlayerController->ClientMessage(L"Server Speed Changed!", FName(), 1);
		}
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
			AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;

			if (PlayerPawn == NULL)
				return;

			std::string SetHealth_ = "sethealth";
			std::string Args;

			if (Message.size() > SetHealth_.size() + 1)
				Args = Message.substr(SetHealth_.size() + 1);

			std::stringstream ss(Args);

			float Health;
			ss >> Health;

			PlayerPawn->SetHealth(Health);

			PlayerController->ClientMessage(L"Health Changed!", FName(), 1);
		}
	}

	if (Message.contains("setshield"))
	{
		if (Message.rfind("setshield", 0) == 0)
		{
			AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;

			if (PlayerPawn == NULL)
				return;

			std::string SetShield_ = "setshield";
			std::string Args;

			if (Message.size() > SetShield_.size() + 1)
				Args = Message.substr(SetShield_.size() + 1);

			std::stringstream ss(Args);

			float Shield;
			ss >> Shield;

			PlayerPawn->SetShield(Shield);

			PlayerController->ClientMessage(L"Shield Changed!", FName(), 1);
		}
	}

	if (Message.contains("launch"))
	{
		if (Message.rfind("launch", 0) == 0)
		{
			AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;

			if (PlayerPawn == NULL)
				return;

			std::string Launch_ = "launch";
			std::string Args;

			if (Message.size() > Launch_.size() + 1)
				Args = Message.substr(Launch_.size() + 1);

			std::stringstream ss(Args);

			float X{}, Y{}, Z{};

			ss >> X >> Y >> Z;

			PlayerPawn->LaunchCharacter(FVector(X, Y, Z), false, false);

			PlayerController->ClientMessage(L"Launched Character!", FName(), 1);
		}
	}

	if (Message.rfind("giveitem", 0) == 0)
	{
		std::string GiveItem_ = "giveitem";
		std::string Args;

		if (Message.size() > GiveItem_.size() + 1)
			Args = Message.substr(GiveItem_.size() + 1);

		std::stringstream ss(Args);

		std::string WeaponId;

		int Count { 1 };

		ss >> WeaponId;

		if (!(ss >> Count))
			Count = 1;

		std::wstring WID(WeaponId.begin(), WeaponId.end());

		UFortWorldItemDefinition* ItemDefinition = Cast<UFortWorldItemDefinition>(Utils::StaticFindObject(WID.c_str(), ANY_PACKAGE));

		if (ItemDefinition == NULL)
		{
			PlayerController->ClientMessage(L"Failed to find item definition! Are you entering the correct WID?", FName(), 1);
			return;
		}

		if (UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition))
		{
			Inventory::GiveItem(PlayerController->WorldInventory, ItemDefinition, Count, Inventory::GetStats(WeaponItemDefinition)->ClipSize);
		}
		else
		{
			Inventory::GiveItem(PlayerController->WorldInventory, ItemDefinition, Count, 0);
		}

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
	if (Pickup != NULL)
		Pickup->SetPickupTarget(PlayerPawn, InFlyTime / PlayerPawn->PickupSpeedMultiplier, InStartDirection);
}

void Player::GivePickupTo(AFortPickup* Pickup, IFortInventoryOwnerInterface* InventoryOwner, bool DestroyAfterPickup)
{
	Originals::GivePickupTo(Pickup, InventoryOwner, DestroyAfterPickup);

	AFortPlayerPawnAthena* PlayerPawn = Cast<AFortPlayerPawnAthena>(Pickup->PickupLocationData.PickupTarget);

	if (PlayerPawn == NULL)
		return;

	AFortPlayerController* PlayerController = Cast<AFortPlayerController>(PlayerPawn->Controller);

	if (PlayerController == NULL)
		return;

	Inventory::GiveItem(PlayerController->WorldInventory, Pickup->PrimaryPickupItemEntry.ItemDefinition, Pickup->PrimaryPickupItemEntry.Count, Pickup->PrimaryPickupItemEntry.LoadedAmmo);
}

void Player::ClientOnPawnDied(AFortPlayerControllerAthena* PlayerController, FFortPlayerDeathReport& DeathReport)
{
	Originals::ClientOnPawnDied(PlayerController, DeathReport);

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;

	if (PlayerPawn == NULL)
		return;

	AFortPlayerStateAthena* KillerPlayerState = Cast<AFortPlayerStateAthena>(DeathReport.KillerPlayerState);

	if (KillerPlayerState == NULL)
		return;

	AFortPlayerPawnAthena* KillerPawn = Cast<AFortPlayerPawnAthena>(DeathReport.KillerPawn);

	if (KillerPawn == NULL)
		return;

	FDeathInfo DeathInfo = FDeathInfo{};

	DeathInfo.bDBNO = PlayerPawn ? PlayerPawn->IsDBNO() : false;
	DeathInfo.bInitialized = true;
	DeathInfo.DeathCause = PlayerState->ToDeathCause(DeathReport.Tags, PlayerPawn ? PlayerPawn->IsDBNO() : false);
	DeathInfo.DeathLocation = PlayerPawn->K2_GetActorLocation();
	DeathInfo.DeathTags = DeathReport.Tags;
	DeathInfo.Downer = KillerPlayerState;
	DeathInfo.FinisherOrDowner = KillerPlayerState;

	PlayerState->DeathInfo = DeathInfo;
	PlayerState->OnRep_DeathInfo();

	if (KillerPlayerState && KillerPlayerState != PlayerState)
	{
		KillerPlayerState->KillScore++;
		KillerPlayerState->OnRep_Kills();

		KillerPlayerState->ClientReportKill(PlayerState);

		if (AFortTeamInfo* PlayerTeam = KillerPlayerState->PlayerTeam)
		{
			for (AController* TeamMember : PlayerTeam->TeamMembers)
			{
				AFortPlayerStateAthena* TeamMemberState = Cast<AFortPlayerStateAthena>(TeamMember->PlayerState);

				if (TeamMemberState == NULL)
					continue;

				TeamMemberState->TeamKillScore++;

				TeamMemberState->OnRep_TeamKillScore();
				TeamMemberState->ClientReportTeamKill(1);
			}
		}
	}

	if (AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState))
	{
		AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);

		if (!GameState->IsRespawningAllowed(PlayerState))
		{
			PlayerController->ClientSendEndBattleRoyaleMatchForPlayer(true, FAthenaRewardResult());

			FAthenaMatchTeamStats TeamStats{};

			if (PlayerState)
			{
				PlayerState->Place = GameState->PlayersLeft;
				PlayerState->OnRep_Place();
			}

			TeamStats.Place = PlayerState->Place;
			TeamStats.TotalPlayers = GameState->TotalPlayers;

			PlayerController->ClientSendMatchStatsForPlayer(FAthenaMatchStats());
			PlayerController->ClientSendTeamStatsForPlayer(TeamStats);

			GameMode->RemoveFromAlivePlayers(PlayerController, KillerPlayerState == PlayerState ? NULL : KillerPlayerState, (AFortPlayerPawn*)DeathReport.KillerPawn, DeathReport.KillerWeapon, PlayerState ? PlayerState->DeathInfo.DeathCause : EDeathCause::Rifle, 0);

			if (KillerPlayerState && KillerPlayerState != PlayerState && KillerPlayerState->Place == 1)
			{
				GameState->WinningTeam = KillerPlayerState->TeamIndex;
				GameState->OnRep_WinningTeam();

				GameState->WinningPlayerState = KillerPlayerState;
				GameState->OnRep_WinningPlayerState();
			}
		}
	}
}

void Player::RemoveAllInventoryItems(AFortPlayerControllerAthena* PlayerController)
{
	PlayerController->WorldInventory->Inventory.ReplicatedEntries.Free();
	PlayerController->WorldInventory->Inventory.ItemInstances.Free();

	PlayerController->WorldInventory->Inventory.MarkArrayDirty();
}

void Player::Setup()
{
	Utils::Virtual<AFortPickupAthena>(0xC7, GivePickupTo, (void**)&Originals::GivePickupTo);

    Utils::Virtual<AFortPlayerControllerAthena>(0x10A, ServerAcknowledgePossession);
	Utils::Virtual<AFortPlayerControllerAthena>(0x209, ServerExecuteInventoryItem);
	Utils::Virtual<AFortPlayerControllerAthena>(0x1C3, ServerPlayEmoteItem);

	Utils::Virtual<AFortPlayerControllerAthena>(0xE08 / 8, ServerCheat);
	Utils::Virtual<AFortPlayerControllerAthena>(0x10E8 / 8, ServerAttemptInventoryDrop);

	Utils::Virtual<AFortPlayerPawnAthena>(0x1E1, ServerSendZiplineState);
	Utils::Virtual<AFortPlayerPawnAthena>(0xEB0 / 8, ServerHandlePickup);

	//Utils::Virtual<UAthenaMarkerComponent>(0x7F, ServerAddMapMarker);
	//Utils::Virtual<UAthenaMarkerComponent>(0x7D, ServerRemoveMapMarker);

	Utils::Virtual<UFortControllerComponent_Aircraft>(0x82, ServerAttemptAircraftJump);

	Utils::Hook(InSDKUtils::GetImageBase() + 0x1E181D0, GetPlayerViewPoint, (void**)&Originals::GetPlayerViewPoint);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x2446300, ClientOnPawnDied, (void**)&Originals::ClientOnPawnDied);

	Utils::Rel32(InSDKUtils::GetImageBase() + 0x154E9E8, RemoveAllInventoryItems);
}