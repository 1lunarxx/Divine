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
		for (const FItemAndCount& Item : GameMode->StartingItems)
		{
			if (Item.Count > 0)
			{
				Inventory::CreateItem(PlayerController->WorldInventory, Item.Item, Item.Count);
			}
		}

		Inventory::CreateItem(PlayerController->WorldInventory, PlayerController->CosmeticLoadoutPC.Pickaxe->WeaponDefinition);
	}
}

void Player::ServerSendZiplineState(AFortPlayerPawn* FortPlayerPawn, FZiplinePawnState& ZiplinePawnState)
{
	FortPlayerPawn->ZiplineState = ZiplinePawnState;
	auto Velocity = 0.f;
	((void (*)(AFortPlayerPawn*))(InSDKUtils::GetImageBase() + 0x1DC5980))(FortPlayerPawn);

	if (ZiplinePawnState.bJumped)
		FortPlayerPawn->LaunchCharacterJump({ Velocity >= -750 ? fminf(Velocity, 750) : -750, Velocity >= -750 ? fminf(Velocity, 750) : -750, 1500 }, false, false, true, true);
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

void Player::Setup()
{
    Utils::Virtual<AFortPlayerControllerAthena>(0x10A, ServerAcknowledgePossession);
	Utils::Virtual<AFortPlayerPawnAthena>(0x1E1, ServerSendZiplineState);
	Utils::Virtual<AFortPlayerControllerAthena>(0x209, ServerExecuteInventoryItem);

	Utils::Virtual<UFortControllerComponent_Aircraft>(0x82, ServerAttemptAircraftJump);

	Utils::Rel32(InSDKUtils::GetImageBase() + 0x154E9E8, RemoveAllInventoryItems);
}