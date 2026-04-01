#include "pch.h"
#include "Misc.h"
#include "Utils.h"
#include "Bots.h"
#include "Configuration.h"

void Misc::TickFlush(UNetDriver* NetDriver)
{
	if (UReplicationDriver* ReplicationDriver = NetDriver->ReplicationDriver)
	{
		ReplicationDriver->ServerReplicateActors();
	}

	if (GetKeyState(VK_F3))
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
		if (UFortServerBotManagerAthena* ServerBotManager = GameMode->ServerBotManager)
		{
			for (const FFortServerBotInfo& PlayerBot : ServerBotManager->PlayerBots)
			{
				AFortAthenaAIBotController* BotController = PlayerBot.BotController;

				if (BotController != NULL)
					Bots::EnterAircraft(BotController, GameState->GetAircraft(0));
			}
		}
	}
}

void Misc::ServerMove(AFortPhysicsPawn* PhysicsPawn, FReplicatedPhysicsPawnState& InState)
{
	printf("sadsaw\n");

	UFortVehicleSkelMeshComponent* RootComponent = Cast<UFortVehicleSkelMeshComponent>(PhysicsPawn->RootComponent);

	if (RootComponent != NULL)
	{
		PhysicsPawn->ReplicatedMovement.AngularVelocity = InState.AngularVelocity;
		PhysicsPawn->ReplicatedMovement.LinearVelocity = InState.LinearVelocity;
		PhysicsPawn->ReplicatedMovement.Location = InState.Translation;
		PhysicsPawn->ReplicatedMovement.Rotation = Utils::FQuatToRot(InState.Rotation);

		PhysicsPawn->OnRep_ReplicatedMovement();

		RootComponent->SetAllPhysicsLinearVelocity(InState.LinearVelocity, false);
		RootComponent->SetAllPhysicsAngularVelocityInRadians(InState.AngularVelocity, false);

		RootComponent->K2_SetWorldRotation(PhysicsPawn->ReplicatedMovement.Rotation, false, nullptr, true);
		RootComponent->K2_SetWorldLocation(InState.Translation, false, nullptr, true);
	}
}

FServicePermissionsMcp* Misc::MatchMakingServicePerms(int64, int64)
{
	FServicePermissionsMcp* Permissions = new FServicePermissionsMcp();
	Permissions->Id = L"ec684b8c687f479fadea3cb2ad83f5c6";

	return Permissions;
}

static int ForceRetTrue()
{
	return true;
}

static float GetMaxTickRate()
{
	return 120.f;
}

void Misc::Setup()
{
	Utils::Hook(InSDKUtils::GetImageBase() + 0x3883CD0, TickFlush, (void**)&Originals::TickFlush);
	Utils::Hook(InSDKUtils::GetImageBase() + 0xE2BF70, SendRequestNow, (void**)&Originals::SendRequestNow);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x1589170, PlacePlayersInAircraft, (void**)&Originals::PlacePlayersInAircraft);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x3B24510, GetMaxTickRate);

	Utils::Virtual<AFortPhysicsPawn>(0x768 / 8, ServerMove);

	if (Configuration::bUseGameSessions)
	{
		Utils::Hook(InSDKUtils::GetImageBase() + 0x8A7580, MatchMakingServicePerms);

		Utils::Hook(InSDKUtils::GetImageBase() + 0x3813C70, ForceRetTrue); // SpawnPlayActor

		Utils::Patch(InSDKUtils::GetImageBase() + 0x1C1A525, 0x85);
	}

	Utils::Hook(InSDKUtils::GetImageBase() + 0x3B76FE0, ForceRetTrue);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x34AF6C0, ForceRetTrue);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x372FB80, ForceRetTrue);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x163DCA0, ForceRetTrue);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x27C47A0, ForceRetTrue);

	Utils::Patch(InSDKUtils::GetImageBase() + 0x19B1660, 0xC3);
	Utils::Patch(InSDKUtils::GetImageBase() + 0x1591C25, 0x85);

	Utils::Rel32(InSDKUtils::GetImageBase() + 0x3B2BF8A, Listen);
}