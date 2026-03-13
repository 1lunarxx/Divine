#include "pch.h"
#include "Bots.h"
#include "Utils.h"

void Bots::CreateAndConfigureNavigationSystem(UAthenaNavSystemConfig* NavSystemConfig, UWorld* World)
{
	NavSystemConfig->bPrioritizeNavigationAroundSpawners = true;
	NavSystemConfig->bAutoSpawnMissingNavData = true;
	NavSystemConfig->bSpawnNavDataInNavBoundsLevel = true;
	NavSystemConfig->bSupportRuntimeNavmeshDisabling = false;
	NavSystemConfig->bAllowAutoRebuild = true;
	NavSystemConfig->bUsesStreamedInNavLevel = true;
	NavSystemConfig->SupportedAgentsMask.bSupportsAgent3 = 1;

	return Originals::CreateAndConfigureNavigationSystem(NavSystemConfig, World);
}

void Bots::OnSafeZoneUpdated(__int64 a1)
{
	*(int*)(a1 + 0x428) = 1;
	return Originals::OnSafeZoneUpdated(a1);
}

void Bots::OnPawnSpawned(AFortAthenaMutator_Bots* Mutator_Bots, AFortPlayerPawn* PlayerPawn)
{
	AFortAthenaAIBotController* BotController = Cast<AFortAthenaAIBotController>(PlayerPawn->Controller);
	if (BotController == NULL)
		return Originals::OnPawnSpawned(Mutator_Bots, PlayerPawn);

	BotController->BehaviorTree = Utils::StaticLoadObject<UBehaviorTree>("/Game/Athena/AI/Phoebe/BehaviorTrees/BT_Phoebe.BT_Phoebe");
	BotController->BlueprintOnBehaviorTreeStarted();
	BotController->RunBehaviorTree(BotController->BehaviorTree);

	UDataTable* Pheobe_Cosmetic_Emotes = Utils::StaticLoadObject<UDataTable>("/Game/Athena/AI/Phoebe/Cosmetic/Pheobe_Cosmetic_Emotes.Pheobe_Cosmetic_Emotes");
	TArray<FFortBotCosmeticItemDataTableRow*> Pheobe_Cosmetic_EmotesRows;

	for (auto& Row : Pheobe_Cosmetic_Emotes->RowMap)
	{
		FFortBotCosmeticItemDataTableRow* TableRow = (FFortBotCosmeticItemDataTableRow*)Row.Value();
		if (TableRow)
			Pheobe_Cosmetic_EmotesRows.Add(TableRow);
	}

	for (auto& Emote : Pheobe_Cosmetic_EmotesRows)
	{
		UAthenaDanceItemDefinition* ItemDef = Cast<UAthenaDanceItemDefinition>(UKismetSystemLibrary::GetObjectFromPrimaryAssetId(Emote->PrimaryAssetId));
		if (ItemDef)
			BotController->CosmeticLoadoutBC.Dances.Add(ItemDef);
	}

	BotController->PlayerBotPawn->OnRep_CosmeticLoadout();

	return Originals::OnPawnSpawned(Mutator_Bots, PlayerPawn);
}

void Bots::EnterAircraft(AFortAthenaAIBotController* BotController, AFortAthenaAircraft* InAircraft)
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
		Blackboard->SetValueAsBool(UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_IsInBus"), true);
		Blackboard->SetValueAsBool(UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_HasEverJumpedFromBusKey"), false);
		Blackboard->SetValueAsBool(UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_HasEverJumpedFromBusAndLandedKey"), false);

		if (AFortPlayerStateZone* PlayerState = Cast<AFortPlayerStateZone>(BotController->PlayerState))
		{
			PlayerState->bHasEverSkydivedFromBusAndLanded = 0;
			PlayerState->bHasEverSkydivedFromBus = 0;

			PlayerState->bInAircraft = 1;
		}
	}
}

void Bots::Patch()
{
	Utils::Hook(InSDKUtils::GetImageBase() + 0x16414A0, OnSafeZoneUpdated, (void**)&Originals::OnSafeZoneUpdated);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x3D875A0, CreateAndConfigureNavigationSystem, (void**)&Originals::CreateAndConfigureNavigationSystem);
}