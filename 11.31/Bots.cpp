#include "pch.h"
#include "Bots.h"
#include "Utils.h"

void Bots::BTTask_Dive_TickTask(UFortAthenaBTTask_Dive* BTTask_Dive, UBehaviorTreeComponent* OwnerComp, unsigned __int8* NodeMemory, float DeltaSeconds)
{
	AActor* Owner = OwnerComp->GetOwner();
	double X = Owner->ReplicatedMovement.Location.X;

	if (X == 0.0)
	{
		return Originals::BTTask_Dive_TickTask(BTTask_Dive, OwnerComp, NodeMemory, DeltaSeconds);
	}

	if (UBlackboardComponent* Blackboard = OwnerComp->BlackboardComp)
	{
		Blackboard->SetValueAsEnum(BTTask_Dive->ExecutionStatusKeyName, (uint8)EExecutionStatus::ExecutionSuccess);
	}

	return Originals::BTTask_Dive_TickTask(BTTask_Dive, OwnerComp, NodeMemory, DeltaSeconds);
}

void Bots::BTTask_Glide_TickTask(UFortAthenaBTTask_Glide* BTTask_Glide, UBehaviorTreeComponent* OwnerComp, unsigned __int8* NodeMemory, double DeltaSeconds)
{
	AActor* Owner = OwnerComp->GetOwner();
	double X = Owner->ReplicatedMovement.Location.X;

	if (X == 0.0)
	{
		return Originals::BTTask_Glide_TickTask(BTTask_Glide, OwnerComp, NodeMemory, DeltaSeconds);
	}

	if (UBlackboardComponent* Blackboard = OwnerComp->BlackboardComp)
	{
		Blackboard->SetValueAsEnum(BTTask_Glide->ExecutionStatusKeyName, (uint8)EExecutionStatus::ExecutionSuccess);
	}

	return Originals::BTTask_Glide_TickTask(BTTask_Glide, OwnerComp, NodeMemory, DeltaSeconds);
}

void Bots::CreateAndConfigureNavigationSystem(UAthenaNavSystemConfig* NavSystemConfig, UWorld* World)
{
	NavSystemConfig->bPrioritizeNavigationAroundSpawners = true;
	NavSystemConfig->bAutoSpawnMissingNavData = true;
	NavSystemConfig->bAllowAutoRebuild = true;
	NavSystemConfig->bSupportRuntimeNavmeshDisabling = false;
	NavSystemConfig->bUsesStreamedInNavLevel = true;

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

	((AFortAIController*)BotController)->bUsingNavMesh = true;

	AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);
	if (GameMode == NULL)
		return Originals::OnPawnSpawned(Mutator_Bots, PlayerPawn);

	BotController->BehaviorTree = Utils::StaticLoadObject<UBehaviorTree>("/Game/Athena/AI/Phoebe/BehaviorTrees/BT_Phoebe.BT_Phoebe");
	BotController->BlueprintOnBehaviorTreeStarted();
	BotController->RunBehaviorTree(BotController->BehaviorTree);

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

void Bots::InitializeForWorld(UAthenaNavSystem* NavigationSystem, UWorld* World, uint8 Mode)
{
	NavigationSystem->SupportedAgentsMask.bSupportsAgent3 = 1;
	NavigationSystem->bAutoCreateNavigationData = true;
	NavigationSystem->bSpawnNavDataInNavBoundsLevel = true;

	void(*Build_)(UNavigationSystemV1 * NavigationSystem) = decltype(Build_)(InSDKUtils::GetImageBase() + 0x3D84600);
	Build_(NavigationSystem);

	return Originals::InitializeForWorld(NavigationSystem, World, Mode);
}

void Bots::Tick()
{
	AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);
	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);

	for (FFortServerBotInfo& PlayerBot : GameMode->ServerBotManager->PlayerBots)
	{
		AFortPlayerStateAthena* BotState = Cast<AFortPlayerStateAthena>(PlayerBot.BotController->PlayerState);

		if (GameState->GamePhase == EAthenaGamePhase::Aircraft && !GameState->bAircraftIsLocked)
		{
			if (!PlayerBot.BotController->PlayerBotPawn->bIsSkydiving && UKismetMathLibrary::RandomBoolWithWeight(0.003f))
			{
				PlayerBot.BotController->PlayerBotPawn->K2_TeleportTo(GameState->GetAircraft(0)->K2_GetActorLocation(), {});
				PlayerBot.BotController->PlayerBotPawn->BeginSkydiving(true);

				PlayerBot.BotController->Blackboard->SetValueAsBool(UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_IsInBus"), false);
				PlayerBot.BotController->Blackboard->SetValueAsBool(UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_HasEverJumpedFromBusKey"), true);

				BotState->bInAircraft = 0;
				BotState->bHasEverSkydivedFromBus = 1;
			}
		}
	}
}

void Bots::Patch()
{
	Utils::Hook(InSDKUtils::GetImageBase() + 0x16414A0, OnSafeZoneUpdated, (void**)&Originals::OnSafeZoneUpdated);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x3D875A0, CreateAndConfigureNavigationSystem, (void**)&Originals::CreateAndConfigureNavigationSystem);

	Utils::Hook(InSDKUtils::GetImageBase() + 0x165CB70, OnPawnSpawned, (void**)&Originals::OnPawnSpawned);

	Utils::Hook(InSDKUtils::GetImageBase() + 0x14A63E0, BTTask_Glide_TickTask, (void**)&Originals::BTTask_Glide_TickTask);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x14A5F50, BTTask_Dive_TickTask, (void**)&Originals::BTTask_Dive_TickTask);

	Utils::Virtual<UAthenaNavSystem>(0x53, InitializeForWorld, (void**)&Originals::InitializeForWorld);
}