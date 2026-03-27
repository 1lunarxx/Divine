#include "pch.h"
#include "Misc.h"
#include "Utils.h"
#include "Bots.h"

void Misc::TickFlush(UNetDriver* NetDriver)
{
	if (UReplicationDriver* ReplicationDriver = NetDriver->ReplicationDriver)
	{
		ReplicationDriver->ServerReplicateActors();
	}

	if (AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode))
	{
		AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);

		if (UFortServerBotManagerAthena* ServerBotManager = GameMode->ServerBotManager)
		{
			for (FFortServerBotInfo& PlayerBot : ServerBotManager->PlayerBots)
			{
				PlayerBot.BotController->Blackboard->SetValueAsEnum(UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_GamePhaseStep"), (uint8)GameState->GamePhaseStep);
				PlayerBot.BotController->Blackboard->SetValueAsEnum(UKismetStringLibrary::Conv_StringToName(L"AIEvaluator_Global_GamePhase"), (uint8)GameState->GamePhase);
			}
		}
	}

	if (GetKeyState(VK_F2))
	{
		UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"startaircraft", nullptr);
	}

	return Originals::TickFlush(NetDriver);
}

void Misc::SendRequestNow(UMcpProfileGroup* McpProfileGroup, const void* HttpRequest, FBaseUrlContext::EContextCredentials ContextCredentials)
{
	return Originals::SendRequestNow(McpProfileGroup, HttpRequest, FBaseUrlContext::EContextCredentials::CXC_Public);
}

bool Misc::Listen()
{
	FName NetDriverDefinition = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver");
	UNetDriver* NetDriver = UEngine::GetEngine()->CreateNetDriver(UWorld::GetWorld(), NetDriverDefinition);

	NetDriver->World = UWorld::GetWorld();
	NetDriver->NetDriverName = NetDriverDefinition;

	FString Error;

	FURL InURL;
	InURL.Port = 7777;

	if (!NetDriver->InitListen(UWorld::GetWorld(), InURL, false, Error))
		return false;

	NetDriver->SetWorld(UWorld::GetWorld());

	UWorld::GetWorld()->NetDriver = NetDriver;

	for (FLevelCollection& LevelCollection : UWorld::GetWorld()->LevelCollections)
	{
		LevelCollection.NetDriver = UWorld::GetWorld()->NetDriver;
	}

	SetConsoleTitleA("Winter | Listening");

	return true;
}

void Misc::PlacePlayersInAircraft(AFortGameStateAthena* GameState, bool a2)
{
	Originals::PlacePlayersInAircraft(GameState, a2);

	if (AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode))
	{
		if (UFortServerBotManagerAthena* BotManager = GameMode->ServerBotManager)
		{
			for (FFortServerBotInfo& PlayerBot : BotManager->PlayerBots)
			{
				AFortAthenaAIBotController* BotController = PlayerBot.BotController;

				if (BotController != NULL)
					Bots::EnterAircraft(BotController, GameState->GetAircraft(0));
			}
		}
	}
}

static int ForceRetTrue()
{
	return 1;
}

void Misc::Setup()
{
	Utils::Hook(InSDKUtils::GetImageBase() + 0x3883CD0, TickFlush, (void**)&Originals::TickFlush);
	Utils::Hook(InSDKUtils::GetImageBase() + 0xE2BF70, SendRequestNow, (void**)&Originals::SendRequestNow);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x1589170, PlacePlayersInAircraft, (void**)&Originals::PlacePlayersInAircraft);

	//Utils::Patch(InSDKUtils::GetImageBase() + 0x32EAAD0, 0xC3);
	Utils::Patch(InSDKUtils::GetImageBase() + 0x19B1660, 0xC3);
	Utils::Patch(InSDKUtils::GetImageBase() + 0x27C47A0, 0xC3);

	Utils::Hook(InSDKUtils::GetImageBase() + 0x372FB80, ForceRetTrue);
	Utils::Hook(InSDKUtils::GetImageBase() + 0xB625A0, ForceRetTrue);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x27E8B00, ForceRetTrue);

	Utils::Hook(InSDKUtils::GetImageBase() + 0x3B76FE0, GetNetMode);

	Utils::Rel32(InSDKUtils::GetImageBase() + 0x3B2BF8A, Listen);
}