#include "pch.h"
#include "Bots.h"
#include "Utils.h"

void TickService(AFortAthenaAIBotController* BotController)
{
	if (BotController == NULL)
		return;

	if (BotController->PlayerBotPawn == NULL)
		return;

	if (AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState))
	{
		if (!GameState->bAircraftIsLocked && !BotController->PlayerBotPawn->bIsSkydiving && UKismetMathLibrary::RandomBoolWithWeight(0.003f))
		{
			BotController->PlayerBotPawn->K2_TeleportTo(GameState->GetAircraft(0)->K2_GetActorLocation(), {});
			BotController->PlayerBotPawn->BeginSkydiving(true);

			if (UBlackboardComponent* Blackboard = BotController->Blackboard)
			{
				Blackboard->SetValueAsBool(UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_IsInBus"), 0);
				Blackboard->SetValueAsBool(UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_HasEverJumpedFromBusKey"), 1);
			}
		}
	}
}

void Bots::CreateAndConfigureNavigationSystem(UAthenaNavSystemConfig* NavSystemConfig, UWorld* World)
{
	NavSystemConfig->bPrioritizeNavigationAroundSpawners = true;
	NavSystemConfig->bAutoSpawnMissingNavData = true;
	NavSystemConfig->bSupportRuntimeNavmeshDisabling = false;
	NavSystemConfig->bAllowAutoRebuild = true;
	NavSystemConfig->bUsesStreamedInNavLevel = true;

	return Originals::CreateAndConfigureNavigationSystem(NavSystemConfig, World);
}

void Bots::OnSafeZoneUpdated(__int64 a1)
{
	*(int*)(a1 + 0x428) = 1;
	return Originals::OnSafeZoneUpdated(a1);
}

void Bots::BallsInTheFan(AFortAthenaAIBotController* BotController)
{
	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(BotController->PlayerState);

	if (PlayerState == NULL)
		return;

	static UDataTable* PhoebeCharacterDataTable = Utils::StaticLoadObject<UDataTable>("/Game/Athena/AI/Phoebe/Cosmetic/CosmeticLibraries/Basic/Phoebe_Cosmetic_Characters_Basic.Phoebe_Cosmetic_Characters_Basic");
	static UDataTable* PhoebeGlidersDataTable = Utils::StaticLoadObject<UDataTable>("/Game/Athena/AI/Phoebe/Cosmetic/CosmeticLibraries/Basic/Phoebe_Cosmetic_Gliders_Basic.Phoebe_Cosmetic_Gliders_Basic");
	static UDataTable* PhoebePickaxesDataTable = Utils::StaticLoadObject<UDataTable>("/Game/Athena/AI/Phoebe/Cosmetic/CosmeticLibraries/Basic/Phoebe_Cosmetic_Pickaxe_Basic.Phoebe_Cosmetic_Pickaxe_Basic");
	static UDataTable* PheobeEmotesDataTable = Utils::StaticLoadObject<UDataTable>("/Game/Athena/AI/Phoebe/Cosmetic/Pheobe_Cosmetic_Emotes.Pheobe_Cosmetic_Emotes");

	TArray<FFortBotCosmeticItemDataTableRow*> PhoebeCharacterDataTableRow;
	TArray<FFortBotCosmeticItemDataTableRow*> PhoebeGlidersDataTableRow;
	TArray<FFortBotCosmeticItemDataTableRow*> PhoebePickaxesDataTableRow;
	TArray<FFortBotCosmeticItemDataTableRow*> PheobeEmotesDataTableRow;

	for (const auto& [RowName, RowValue] : PhoebeCharacterDataTable->RowMap)
	{
		if (RowValue == NULL)
			continue;

		FFortBotCosmeticItemDataTableRow* Cosmetic = (FFortBotCosmeticItemDataTableRow*)RowValue;
		if (Cosmetic)
			PhoebeCharacterDataTableRow.Add(Cosmetic);
	}

	for (const auto& [RowName, RowValue] : PhoebeGlidersDataTable->RowMap)
	{
		if (RowValue == NULL)
			continue;

		FFortBotCosmeticItemDataTableRow* Cosmetic = (FFortBotCosmeticItemDataTableRow*)RowValue;
		if (Cosmetic)
			PhoebeGlidersDataTableRow.Add(Cosmetic);
	}

	for (const auto& [RowName, RowValue] : PhoebePickaxesDataTable->RowMap)
	{
		if (RowValue == NULL)
			continue;

		FFortBotCosmeticItemDataTableRow* Cosmetic = (FFortBotCosmeticItemDataTableRow*)RowValue;
		if (Cosmetic)
			PhoebePickaxesDataTableRow.Add(Cosmetic);
	}

	for (const auto& [RowName, RowValue] : PheobeEmotesDataTable->RowMap)
	{
		if (RowValue == NULL)
			continue;

		FFortBotCosmeticItemDataTableRow* Cosmetic = (FFortBotCosmeticItemDataTableRow*)RowValue;
		if (Cosmetic)
			PheobeEmotesDataTableRow.Add(Cosmetic);
	}

	int32 CharacterIndex = UKismetMathLibrary::RandomIntegerInRange(0, PhoebeCharacterDataTableRow.Num() - 1);
	int32 GliderIndex = UKismetMathLibrary::RandomIntegerInRange(0, PhoebeGlidersDataTableRow.Num() - 1);
	int32 PickaxeIndex = UKismetMathLibrary::RandomIntegerInRange(0, PhoebePickaxesDataTableRow.Num() - 1);

	FFortBotCosmeticItemDataTableRow* CharacterRow = PhoebeCharacterDataTableRow[CharacterIndex];
	FFortBotCosmeticItemDataTableRow* GliderRow = PhoebeGlidersDataTableRow[GliderIndex];
	FFortBotCosmeticItemDataTableRow* PickaxeRow = PhoebePickaxesDataTableRow[PickaxeIndex];

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

		BotController->PlayerBotPawn->CosmeticLoadout.Character = CharacterItemDefinition;
	}

	if (UKismetMathLibrary::RandomBoolWithWeight(PickaxeRow->Weight))
	{
		UAthenaPickaxeItemDefinition* PickaxeItemDefinition = (UAthenaPickaxeItemDefinition*)UKismetSystemLibrary::GetObjectFromPrimaryAssetId(PickaxeRow->PrimaryAssetId);
		BotController->PlayerBotPawn->CosmeticLoadout.Pickaxe = PickaxeItemDefinition;
	}

	if (UKismetMathLibrary::RandomBoolWithWeight(GliderRow->Weight))
	{
		UAthenaGliderItemDefinition* GliderItemDefinition = (UAthenaGliderItemDefinition*)UKismetSystemLibrary::GetObjectFromPrimaryAssetId(GliderRow->PrimaryAssetId);
		BotController->PlayerBotPawn->CosmeticLoadout.Glider = GliderItemDefinition;
	}

	for (auto& Emote : PheobeEmotesDataTableRow)
	{
		UAthenaDanceItemDefinition* ItemDef = Cast<UAthenaDanceItemDefinition>(UKismetSystemLibrary::GetObjectFromPrimaryAssetId(Emote->PrimaryAssetId));
		if (ItemDef)
			BotController->PlayerBotPawn->CosmeticLoadout.Dances.Add(ItemDef);
	}

	BotController->PlayerBotPawn->OnRep_CosmeticLoadout();

}

void Bots::OnPawnSpawned(AFortAthenaMutator_Bots* Mutator_Bots, AFortPlayerPawn* PlayerPawn)
{
	AFortAthenaAIBotController* BotController = Cast<AFortAthenaAIBotController>(PlayerPawn->Controller);
	if (BotController == NULL)
		return Originals::OnPawnSpawned(Mutator_Bots, PlayerPawn);

	AFortPlayerStateAthena* BotState = Cast<AFortPlayerStateAthena>(BotController->PlayerState);
	if (BotState == NULL)
		return Originals::OnPawnSpawned(Mutator_Bots, PlayerPawn);

	AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);
	if (GameMode == NULL)
		return Originals::OnPawnSpawned(Mutator_Bots, PlayerPawn);

	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);
	if (GameState == NULL)
		return Originals::OnPawnSpawned(Mutator_Bots, PlayerPawn);

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

		if (UFortAthenaAIBotLootingDigestedSkillSet* LootingSkillSet = Cast<UFortAthenaAIBotLootingDigestedSkillSet>(SkillSet))
			BotController->CacheLootingSkillSet = LootingSkillSet;
	}

	BallsInTheFan(BotController);

	BotController->BehaviorTree = Utils::StaticLoadObject<UBehaviorTree>("/Game/Athena/AI/Phoebe/BehaviorTrees/BT_Phoebe.BT_Phoebe");
	BotController->RunBehaviorTree(BotController->BehaviorTree);

	BotController->BlueprintOnBehaviorTreeStarted();
	BotController->BrainComponent->RestartLogic();

	BotController->UseBlackboard(BotController->Blackboard->BlackboardAsset, &BotController->Blackboard);
	BotController->OnUsingBlackBoard(BotController->Blackboard, BotController->Blackboard->BlackboardAsset);

	if (UKismetMathLibrary::RandomBool())
	{
		BotController->Blackboard->SetValueAsEnum(UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_WarmupPlayEmote_ExecutionStatus"), (uint8)EExecutionStatus::ExecutionAllowed);
	}
	else
	{
		BotController->Blackboard->SetValueAsEnum(UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_WarmupLootAndShoot_ExecutionStatus"), (uint8)EExecutionStatus::ExecutionAllowed);
	}

	uint8 TeamIndex = BotState->TeamIndex;

	BotState->TeamIndex = GameState->PlayerBotsLeft;
	BotState->OnRep_TeamIndex(TeamIndex);

	BotState->SquadId = BotState->TeamIndex - 3;
	BotState->OnRep_SquadId();

	return Originals::OnPawnSpawned(Mutator_Bots, PlayerPawn);
}

void Bots::InitializeForWorld(UAthenaNavSystem* NavigationSystem, UWorld* World, uint8 Mode)
{
	NavigationSystem->SupportedAgentsMask.bSupportsAgent3 = 1;
	NavigationSystem->bAutoCreateNavigationData = true;
	NavigationSystem->bSpawnNavDataInNavBoundsLevel = true;

	return Originals::InitializeForWorld(NavigationSystem, World, Mode);
}

void Bots::EnterAircraft(AFortAthenaAIBotController* BotController)
{
	if (UFortServerBotManagerAthena* BotManager = BotController->CachedBotManager)
	{
		const float ThankBusDriverProbability = Utils::EvaluateScalableFloat(BotManager->ThankBusDriverProbability);

		if (UKismetMathLibrary::RandomFloat() <= ThankBusDriverProbability)
		{
			const float ThankBusDriverMaxTime = Utils::EvaluateScalableFloat(BotManager->ThankBusDriverMaxTime);
			const float ThankBusDriverMinTime = Utils::EvaluateScalableFloat(BotManager->ThankBusDriverMinTime);

			const float ThankBusDriverTime = UKismetMathLibrary::FMax(UKismetMathLibrary::RandomFloatInRange(ThankBusDriverMinTime, ThankBusDriverMaxTime), 0.f);

			FLatentActionInfo LatentInfo{};
			LatentInfo.Linkage = 0;
			LatentInfo.UUID = UKismetMathLibrary::RandomFloat();
			LatentInfo.ExecutionFunction = UKismetStringLibrary::Conv_StringToName(L"ThankBusDriver");
			LatentInfo.CallbackTarget = BotController;

			UKismetSystemLibrary::Delay(UWorld::GetWorld(), ThankBusDriverTime, LatentInfo);
		}
	}

	if (UBlackboardComponent* Blackboard = BotController->Blackboard)
	{
		Blackboard->SetValueAsBool(UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_IsInBus"), 1);
		Blackboard->SetValueAsBool(UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_HasEverJumpedFromBusKey"), 0);
	}
}

void Bots::Tick()
{
	if (AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode))
	{
		AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);

		if (UFortServerBotManagerAthena* BotManager = GameMode->ServerBotManager)
		{
			for (FFortServerBotInfo& PlayerBot : BotManager->PlayerBots)
			{
				AFortAthenaAIBotController* BotController = PlayerBot.BotController;
				if (BotController == NULL)
					continue;

				BotController->Blackboard->SetValueAsEnum(UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_GamePhase"), (uint8)GameState->GamePhase);
				BotController->Blackboard->SetValueAsEnum(UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_GamePhaseStep"), (uint8)GameState->GamePhaseStep);

				switch (GameState->GamePhase)
				{
				case EAthenaGamePhase::Aircraft:
					TickService(BotController);
					break;
				}
			}
		}
	}
}

void Bots::Patch()
{
	Utils::Hook(InSDKUtils::GetImageBase() + 0x16414A0, OnSafeZoneUpdated, (void**)&Originals::OnSafeZoneUpdated);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x3D875A0, CreateAndConfigureNavigationSystem, (void**)&Originals::CreateAndConfigureNavigationSystem);

	Utils::Hook(InSDKUtils::GetImageBase() + 0x165CB70, OnPawnSpawned, (void**)&Originals::OnPawnSpawned);

	Utils::Virtual<UAthenaNavSystem>(0x50, InitializeForWorld, (void**)&Originals::InitializeForWorld);
}