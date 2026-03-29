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

void Player::ServerPlayEmoteItem(AFortPlayerControllerAthena* Controller, UFortItemDefinition* EmoteAsset, float)
{
	auto Pawn = Controller->MyFortPawn;
	static UClass* AbilityClass = Utils::StaticLoadObject<UClass>("/Game/Abilities/Emotes/GAB_Emote_Generic.GAB_Emote_Generic_C");
	FGameplayAbilitySpec AbilitySpec{};
	using AbilitySpecCtor_t = void(*)(SDK::FGameplayAbilitySpec*, SDK::UGameplayAbility*, int, int, SDK::UObject*);
	using GiveAbilityOnce_t = FGameplayAbilitySpecHandle(*)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec);
	static AbilitySpecCtor_t AbilitySpecCtor = reinterpret_cast<AbilitySpecCtor_t>((uintptr_t)GetModuleHandleW(nullptr) + 0xB9AF40);
	static GiveAbilityOnce_t GiveAbilityAndActivateOnce = reinterpret_cast<GiveAbilityOnce_t>((uintptr_t)GetModuleHandleW(nullptr) + 0xB76F90);

	auto PlayerState = reinterpret_cast<AFortPlayerStateAthena*>(Controller->PlayerState);
	AbilitySpecCtor(&AbilitySpec, DefaultAbility, 1, -1, EmoteAsset);
	auto ASC = PlayerState->AbilitySystemComponent;
	UClass* DanceAbility = NULL;

	if (!Controller || !EmoteAsset)
		return;

	if (!PlayerState)
		return;

	if (UAthenaDanceItemDefinition* DanceItemDefinition = Cast< UAthenaDanceItemDefinition>(EmoteAsset))
	{
		Pawn->bMovingEmote = DanceItemDefinition->bMovingEmote;
		Pawn->EmoteWalkSpeed = DanceItemDefinition->WalkForwardSpeed;
		Pawn->bMovingEmoteForwardOnly = DanceItemDefinition->bMoveForwardOnly;

		DanceAbility = DanceItemDefinition->CustomDanceAbility.Get() ? DanceItemDefinition->CustomDanceAbility.Get() : StaticFindObject<UClass>("/Game/Abilities/Emotes/GAB_Emote_Generic.GAB_Emote_Generic_C");
	}
	else 	if (UAthenaSprayItemDefinition* SprayItemDefinition = Cast<UAthenaSprayItemDefinition>(EmoteAsset))
	{
		static UClass* SprayAbilityClass = Utils::StaticFindObject<UClass>("/Game/Abilities/Sprays/GAB_Spray_Generic.GAB_Spray_Generic_C");
		DanceAbility = SprayAbilityClass;
	}
	GiveAbilityAndActivateOnce(ASC, &AbilitySpec.Handle, AbilitySpec);
}

void Player::ServerSendZiplineState(AFortPlayerPawn* PlayerPawn, FZiplinePawnState& InZiplineState)
{
	PlayerPawn->ZiplineState = InZiplineState;
	PlayerPawn->OnRep_ZiplineState();

	auto Velocity = 0.f;

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

void Player::Setup()
{
    Utils::Virtual<AFortPlayerControllerAthena>(0x10A, ServerAcknowledgePossession);
	Utils::Virtual<AFortPlayerControllerAthena>(0x209, ServerExecuteInventoryItem);
	Utils::Virtual<AFortPlayerControllerAthena>(0x1C3, ServerPlayEmoteItem);
	Utils::Virtual<AFortPlayerPawnAthena>(0x1E1, ServerSendZiplineState);

	Utils::Virtual<UFortControllerComponent_Aircraft>(0x82, ServerAttemptAircraftJump);

	Utils::Rel32(InSDKUtils::GetImageBase() + 0x154E9E8, RemoveAllInventoryItems);
}