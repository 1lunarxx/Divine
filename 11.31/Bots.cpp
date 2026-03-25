#include "pch.h"
#include "Bots.h"
#include "Utils.h"
#include "Configuration.h"
#include "Inventory.h"

void Bots::InitializeMMRInfos(AFortAthenaMutator_Bots* BotMutator)
{
	*(uint8*)((__int64)(BotMutator) + 0x428) = 1; // Match Assignment

	return Originals::InitializeMMRInfos(BotMutator);
}

void Balls_Improper(AFortAthenaAIBotController* BotController)
{
	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(BotController->PlayerState);

	if (PlayerState == NULL)
		return;

	AFortPlayerPawnAthena* PlayerPawn = Cast<AFortPlayerPawnAthena>(BotController->PlayerBotPawn);

	if (PlayerState == NULL)
		return;

	static UDataTable* PheobeEmotes = Utils::StaticLoadObject<UDataTable>("/Game/Athena/AI/Phoebe/Cosmetic/Pheobe_Cosmetic_Emotes.Pheobe_Cosmetic_Emotes");

	for (const auto& [RowName, RowValue] : PheobeEmotes->RowMap)
	{
		FFortBotCosmeticItemDataTableRow* TableRow = (FFortBotCosmeticItemDataTableRow*)RowValue;

		if (TableRow != NULL)
		{
			UAthenaDanceItemDefinition* Dance = (UAthenaDanceItemDefinition*)UKismetSystemLibrary::GetObjectFromPrimaryAssetId(TableRow->PrimaryAssetId);

			if (Dance)
			{
				BotController->CosmeticLoadoutBC.Dances.Add(Dance);
			}
		}
	}

	static UDataTable* PhoebeCharacters = Utils::StaticLoadObject<UDataTable>("/Game/Athena/AI/Phoebe/Cosmetic/CosmeticLibraries/Basic/Phoebe_Cosmetic_Characters_Basic.Phoebe_Cosmetic_Characters_Basic");
	static UDataTable* PhoebeGliders = Utils::StaticLoadObject<UDataTable>("/Game/Athena/AI/Phoebe/Cosmetic/CosmeticLibraries/Basic/Phoebe_Cosmetic_Gliders_Basic.Phoebe_Cosmetic_Gliders_Basic");
	static UDataTable* PhoebePickaxe = Utils::StaticLoadObject<UDataTable>("/Game/Athena/AI/Phoebe/Cosmetic/CosmeticLibraries/Basic/Phoebe_Cosmetic_Pickaxe_Basic.Phoebe_Cosmetic_Pickaxe_Basic");

	TArray<FFortBotCosmeticItemDataTableRow*> PhoebeCharactersRows;
	TArray<FFortBotCosmeticItemDataTableRow*> PhoebeGlidersRows;
	TArray<FFortBotCosmeticItemDataTableRow*> PhoebePickaxeRows;

	for (auto& Row : PhoebeCharacters->RowMap)
	{
		FFortBotCosmeticItemDataTableRow* TableRow = (FFortBotCosmeticItemDataTableRow*)Row.Value();
		if (TableRow)
			PhoebeCharactersRows.Add(TableRow);
	}

	for (auto& Row : PhoebeGliders->RowMap)
	{
		FFortBotCosmeticItemDataTableRow* TableRow = (FFortBotCosmeticItemDataTableRow*)Row.Value();
		if (TableRow)
			PhoebeGlidersRows.Add(TableRow);
	}

	for (auto& Row : PhoebePickaxe->RowMap)
	{
		FFortBotCosmeticItemDataTableRow* TableRow = (FFortBotCosmeticItemDataTableRow*)Row.Value();
		if (TableRow)
			PhoebePickaxeRows.Add(TableRow);
	}

	int32 CharacterIndex = UKismetMathLibrary::RandomIntegerInRange(0, PhoebeCharactersRows.Num() - 1);
	int32 GliderIndex = UKismetMathLibrary::RandomIntegerInRange(0, PhoebeGlidersRows.Num() - 1);
	int32 PickaxeIndex = UKismetMathLibrary::RandomIntegerInRange(0, PhoebePickaxeRows.Num() - 1);

	FFortBotCosmeticItemDataTableRow* CharacterRow = PhoebeCharactersRows[CharacterIndex];
	FFortBotCosmeticItemDataTableRow* GliderRow = PhoebeGlidersRows[GliderIndex];
	FFortBotCosmeticItemDataTableRow* PickaxeRow = PhoebePickaxeRows[PickaxeIndex];

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

		PlayerPawn->CosmeticLoadout.Character = CharacterItemDefinition;
	}

	if (UKismetMathLibrary::RandomBoolWithWeight(GliderRow->Weight))
	{
		UAthenaGliderItemDefinition* GliderItemDefinition = (UAthenaGliderItemDefinition*)UKismetSystemLibrary::GetObjectFromPrimaryAssetId(GliderRow->PrimaryAssetId);
		PlayerPawn->CosmeticLoadout.Glider = GliderItemDefinition;
	}

	if (UKismetMathLibrary::RandomBoolWithWeight(PickaxeRow->Weight))
	{
		UAthenaPickaxeItemDefinition* PickaxeItemDefinition = (UAthenaPickaxeItemDefinition*)UKismetSystemLibrary::GetObjectFromPrimaryAssetId(PickaxeRow->PrimaryAssetId);
		PlayerPawn->CosmeticLoadout.Pickaxe = PickaxeItemDefinition;
	}

	PlayerPawn->OnRep_CosmeticLoadout();
}

void Bots::OnBotSpawned(AFortAthenaMutator_Bots* BotMutator, AFortPlayerPawn* PlayerPawn)
{
	AFortAthenaAIBotController* BotController = Cast<AFortAthenaAIBotController>(PlayerPawn->Controller);

	if (BotController == NULL)
		return Originals::OnBotSpawned(BotMutator, PlayerPawn);

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

		if (UFortAthenaAIBotLootingDigestedSkillSet* LootSkill = Cast<UFortAthenaAIBotLootingDigestedSkillSet>(SkillSet))
			BotController->CacheLootingSkillSet = LootSkill;
	}

	BotController->BotSkillSetClasses.Add(UFortAthenaAIBotWarmupSkillSet::StaticClass());

	Balls_Improper(BotController);

	BotController->BehaviorTree = Utils::StaticLoadObject<UBehaviorTree>("/Game/Athena/AI/Phoebe/BehaviorTrees/BT_Phoebe_Warmup.BT_Phoebe_Warmup");
	BotController->RunBehaviorTree(BotController->BehaviorTree);

	return Originals::OnBotSpawned(BotMutator, PlayerPawn);
}

void Bots::InitializeForWorld(UNavigationSystemV1* NavigationSystem, UWorld* World, EFNavigationSystemRunMode Mode)
{
	NavigationSystem->bAutoCreateNavigationData = true;
	NavigationSystem->bSpawnNavDataInNavBoundsLevel = true;
	NavigationSystem->SupportedAgentsMask.bSupportsAgent3 = 1;

	return Originals::InitializeForWorld(NavigationSystem, World, Mode);
}

void Bots::CreateAndConfigureNavigationSystem(UNavigationSystemConfig* NavigationSystemConfig, UWorld* World)
{
	UAthenaNavSystemConfig* NavSystemConfig = Cast<UAthenaNavSystemConfig>(NavigationSystemConfig);

	NavSystemConfig->bPrioritizeNavigationAroundSpawners = true;
	NavSystemConfig->bSupportRuntimeNavmeshDisabling = false;
	NavSystemConfig->bAutoSpawnMissingNavData = true;
	NavSystemConfig->bAllowAutoRebuild = true;
	NavSystemConfig->bUsesStreamedInNavLevel = true;

	return Originals::CreateAndConfigureNavigationSystem(NavigationSystemConfig, World);
}

void Bots::EnterAircraft(AFortAthenaAIBotController* BotController, AFortAthenaAircraft* InAircraft)
{
	BotController->BehaviorTree = Utils::StaticLoadObject<UBehaviorTree>("/Game/Athena/AI/Phoebe/BehaviorTrees/BT_Phoebe.BT_Phoebe");
	BotController->RunBehaviorTree(BotController->BehaviorTree);

	if (UFortControllerComponent_Aircraft* AircraftComponent = Cast<UFortControllerComponent_Aircraft>(BotController->GetComponentByClass(UFortControllerComponent_Aircraft::StaticClass())))
	{
		printf("yes\n");

		AircraftComponent->EnterAircraft(InAircraft);
	}

	if (UFortServerBotManagerAthena* BotManager = BotController->CachedBotManager)
	{
		const float ThankBusDriverProbability = BotManager->ThankBusDriverProbability.Evaluate();

		if (UKismetMathLibrary::RandomFloat() <= ThankBusDriverProbability)
		{
			const float ThankBusDriverMaxTime = BotManager->ThankBusDriverMaxTime.Evaluate();
			const float ThankBusDriverMinTime = BotManager->ThankBusDriverMinTime.Evaluate();

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
		const FName IsInBusKey = UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_IsInBus");
		const FName HasEverJumpedFromBusKey = UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_HasEverJumpedFromBusKey");
		const FName HasEverJumpedFromBusAndLandedKey = UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_HasEverJumpedFromBusKey");

		Blackboard->SetValueAsBool(IsInBusKey, 1);
		Blackboard->SetValueAsBool(HasEverJumpedFromBusKey, 0);
		Blackboard->SetValueAsBool(HasEverJumpedFromBusAndLandedKey, 0);

		if (AFortPlayerStateZone* PlayerState = Cast<AFortPlayerStateZone>(BotController->PlayerState))
		{
			PlayerState->bHasEverSkydivedFromBus = 0;
			PlayerState->bHasEverSkydivedFromBusAndLanded = 0;
		}
	}
}

void Bots::Setup()
{
	Utils::Hook(InSDKUtils::GetImageBase() + 0x1654BB0, InitializeMMRInfos, (void**)&Originals::InitializeMMRInfos);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x165CB70, OnBotSpawned, (void**)&Originals::OnBotSpawned);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x3D875A0, CreateAndConfigureNavigationSystem, (void**)&Originals::CreateAndConfigureNavigationSystem);

	Utils::Virtual<UAthenaNavSystem>(0x50, InitializeForWorld, (void**)&Originals::InitializeForWorld);
}