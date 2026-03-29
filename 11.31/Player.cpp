#include "pch.h"
#include "Player.h"
#include "Utils.h"
#include "Inventory.h"
#include "SDK/SDK/GAB_AthenaDBNO_classes.hpp"

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

void Player::ServerPlayEmoteItem(AFortPlayerControllerAthena* PlayerController, UFortItemDefinition* EmoteAsset, float EmoteRandomNumber)
{
	AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;

	if (PlayerPawn == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	if (EmoteAsset == NULL)
		return;

	static UClass* AbilityClass = NULL;

	if (UAthenaDanceItemDefinition* DanceItemDefinition = Cast< UAthenaDanceItemDefinition>(EmoteAsset))
	{
		AbilityClass = DanceItemDefinition->CustomDanceAbility.Get() ? DanceItemDefinition->CustomDanceAbility.Get() : Utils::StaticFindObject<UClass>("/Game/Abilities/Emotes/GAB_Emote_Generic.GAB_Emote_Generic_C");

		PlayerPawn->bMovingEmote = DanceItemDefinition->bMovingEmote;
		PlayerPawn->EmoteWalkSpeed = DanceItemDefinition->WalkForwardSpeed;
		PlayerPawn->bMovingEmoteForwardOnly = DanceItemDefinition->bMoveForwardOnly;
	}
	else if (UAthenaSprayItemDefinition* SprayItemDefinition = Cast<UAthenaSprayItemDefinition>(EmoteAsset))
	{
		AbilityClass = Utils::StaticFindObject<UClass>("/Game/Abilities/Sprays/GAB_Spray_Generic.GAB_Spray_Generic_C");
	}

	FGameplayAbilitySpec AbilitySpec{};

	static void(*ConstructSpec)(FGameplayAbilitySpec*, UGameplayAbility*, int, int, UObject*) = decltype(ConstructSpec)(InSDKUtils::GetImageBase() + 0xB9AF40);
	static void(*GiveAbilityAndActivateOnce)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec*) = decltype(GiveAbilityAndActivateOnce)(InSDKUtils::GetImageBase() + 0xB76F90);

	ConstructSpec(&AbilitySpec, Cast<UGameplayAbility>(AbilityClass->ClassDefaultObject), 1, -1, EmoteAsset);
	GiveAbilityAndActivateOnce(PlayerState->AbilitySystemComponent, &AbilitySpec.Handle, &AbilitySpec);
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

void Player::MovingEmoteStopped(AFortPawn* Pawn)
{
	Pawn->bMovingEmote = false;
	return Originals::MovingEmoteStopped(Pawn);
}

void Player::ServerReviveFromDBNO(AFortPlayerPawnAthena* PlayerPawn, AController* EventInstigator)
{
	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(PlayerPawn->Controller);

	if (PlayerController == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	FGameplayEventData EventData = FGameplayEventData{};

	EventData.EventTag = PlayerPawn->EventReviveTag;
	EventData.ContextHandle = PlayerState->AbilitySystemComponent->MakeEffectContext();
	EventData.Instigator = EventInstigator;
	EventData.Target = PlayerPawn;
	EventData.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(PlayerPawn);
	EventData.TargetTags = PlayerPawn->GameplayTags;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(PlayerPawn, PlayerPawn->EventReviveTag, EventData);

	for (FGameplayAbilitySpec& AbilitySpec : PlayerState->AbilitySystemComponent->ActivatableAbilities.Items)
	{
		if (AbilitySpec.Ability->Class == UGAB_AthenaDBNO_C::StaticClass())
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

	PlayerPawn->DBNORevivalStacking = 0;

	PlayerPawn->SetHealth(30);

	PlayerController->RespawnPlayerAfterDeath(false);
	PlayerController->ClientOnPawnRevived(EventInstigator);
}

void Player::Setup()
{
    Utils::Virtual<AFortPlayerControllerAthena>(0x10A, ServerAcknowledgePossession);
	Utils::Virtual<AFortPlayerControllerAthena>(0x209, ServerExecuteInventoryItem);
	Utils::Virtual<AFortPlayerControllerAthena>(0x1C3, ServerPlayEmoteItem);

	Utils::Virtual<AFortPlayerPawnAthena>(0x1E1, ServerSendZiplineState);
	Utils::Virtual<AFortPlayerPawnAthena>(0x1BF, ServerReviveFromDBNO);

	Utils::Virtual<UFortControllerComponent_Aircraft>(0x82, ServerAttemptAircraftJump);

	Utils::Hook(InSDKUtils::GetImageBase() + 0xBE8270, MovingEmoteStopped, (void**)&Originals::MovingEmoteStopped);
	Utils::Rel32(InSDKUtils::GetImageBase() + 0x154E9E8, RemoveAllInventoryItems);
}