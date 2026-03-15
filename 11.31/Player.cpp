#include "pch.h"
#include "Player.h"
#include "Utils.h"

void Player::ServerAcknowledgePossession(AFortPlayerControllerAthena* _this, APawn* P)
{
    _this->AcknowledgedPawn = P;

    AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(_this->PlayerState);

    UFortKismetLibrary::UpdatePlayerCustomCharacterPartsVisualization(PlayerState);
    PlayerState->OnRep_CharacterData();

    TScriptInterface<IAbilitySystemInterface> AbilityInterface{};
    AbilityInterface.ObjectPointer = PlayerState;
    AbilityInterface.InterfacePointer = Utils::GetInterface<IAbilitySystemInterface>(PlayerState);

    static void (*InitializePlayerGameplayAbilities)(void*) = decltype(InitializePlayerGameplayAbilities)(InSDKUtils::GetImageBase() + 0x19D7550);
    InitializePlayerGameplayAbilities(AbilityInterface.InterfacePointer);
}

void Player::ServerExecuteInventoryItem(AFortPlayerController* PlayerController, FGuid& ItemGuid)
{
	FFortItemEntry* ItemEntry = nullptr;

	for (FFortItemEntry& ReplicatedEntry : PlayerController->WorldInventory->Inventory.ReplicatedEntries)
	{
		if (ReplicatedEntry.ItemGuid == ItemGuid)
		{
			ItemEntry = &ReplicatedEntry;
			break;
		}
	}

	if (ItemEntry == NULL)
		return;

	if (PlayerController->MyFortPawn)
	{
		if (UFortItemDefinition* ItemDefinition = ItemEntry->ItemDefinition)
		{
			if (ItemEntry->ItemDefinition->IsA(UFortGadgetItemDefinition::StaticClass()))
			{
				ItemDefinition = ((UFortGadgetItemDefinition*)ItemDefinition)->GetWeaponItemDefinition();
			}

			PlayerController->MyFortPawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)ItemDefinition, ItemGuid);
		}
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

void Player::Patch()
{
    Utils::Virtual<AFortPlayerControllerAthena>(0x10A, ServerAcknowledgePossession);
	Utils::Virtual<AFortPlayerControllerAthena>(0x209, ServerExecuteInventoryItem);
	Utils::Virtual<UFortControllerComponent_Aircraft>(0x82, ServerAttemptAircraftJump);
}