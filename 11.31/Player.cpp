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

		Inventory::CreateItem(PlayerController->WorldInventory, PlayerController->CosmeticLoadoutPC.Pickaxe->WeaponDefinition);
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

void Player::ForceLaunchPlayerZiplining(AFortPlayerPawn* _this)
{
	FVector CurrentVelocity = _this->GetVelocity();

	// how else would i do this?

	float ZiplineJumpDampening_ = 0.f;
	float ZiplineJumpStrength_ = 0.f;

	UFortKismetLibrary::EvaluateCurveTableRow(_this->ZiplineJumpDampening, 0.f, (float*)&ZiplineJumpDampening_, FString());
	UFortKismetLibrary::EvaluateCurveTableRow(_this->ZiplineJumpStrength, 0.f, (float*)&ZiplineJumpStrength_, FString());

	CurrentVelocity.X *= ZiplineJumpDampening_;
	CurrentVelocity.Y *= ZiplineJumpDampening_;

	CurrentVelocity.Z = ZiplineJumpStrength_;

	_this->LaunchCharacterJump(CurrentVelocity, false, false, false, true);
}

void Player::ServerSendZiplineState(AFortPlayerPawn* PlayerPawn, FZiplinePawnState& InZiplineState)
{
	PlayerPawn->ZiplineState = InZiplineState;

	if (!InZiplineState.bIsZiplining)
	{
		if (InZiplineState.bJumped)
		{
			ForceLaunchPlayerZiplining(PlayerPawn);
		}
	}

	PlayerPawn->OnRep_ZiplineState();
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

void Player::ServerAddMapMarker(UAthenaMarkerComponent* MarkerComponent, FFortClientMarkerRequest& MarkerRequest)
{
	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(MarkerComponent->GetOwner());

	if (PlayerController == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	FFortWorldMarkerData WorldMarkerData;
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

// TODO: Rewrite

void Player::ServerAttemptInventoryDrop(AFortPlayerController* PlayerController, FGuid& ItemGuid, int32 Count, bool bTrash)
{
	AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;

	if (PlayerPawn == NULL)
		return;
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

	Inventory::CreateItem(PlayerController->WorldInventory, Pickup->PrimaryPickupItemEntry.ItemDefinition, Pickup->PrimaryPickupItemEntry.Count, Pickup->PrimaryPickupItemEntry.LoadedAmmo);
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
	if (PlayerController->WorldInventory != NULL)
	{
		PlayerController->WorldInventory->Inventory.ReplicatedEntries.Free();
		PlayerController->WorldInventory->Inventory.ItemInstances.Free();

		PlayerController->WorldInventory->Inventory.MarkArrayDirty();
	}
}

void Player::Setup()
{
	Utils::Virtual<AFortPickupAthena>(0x638 / 8, GivePickupTo, (void**)&Originals::GivePickupTo);

    Utils::Virtual<AFortPlayerControllerAthena>(0x850 / 8, ServerAcknowledgePossession);
	Utils::Virtual<AFortPlayerControllerAthena>(0x1048 / 8, ServerExecuteInventoryItem);
	Utils::Virtual<AFortPlayerControllerAthena>(0xE18 / 8, ServerPlayEmoteItem);

	Utils::Virtual<AFortPlayerControllerAthena>(0x10E8 / 8, ServerAttemptInventoryDrop);

	Utils::Virtual<AFortPlayerPawnAthena>(0xF08 / 8, ServerSendZiplineState);
	Utils::Virtual<AFortPlayerPawnAthena>(0xEB0 / 8, ServerHandlePickup);

	Utils::Virtual<UFortControllerComponent_Aircraft>(0x410 / 8, ServerAttemptAircraftJump);

	Utils::Hook(InSDKUtils::GetImageBase() + 0x1E181D0, GetPlayerViewPoint, (void**)&Originals::GetPlayerViewPoint);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x2446300, ClientOnPawnDied, (void**)&Originals::ClientOnPawnDied);

	Utils::Rel32(InSDKUtils::GetImageBase() + 0x154E9E8, RemoveAllInventoryItems);
}