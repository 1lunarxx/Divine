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

void Misc::ServerAddMapMarker(UAthenaMarkerComponent* MarkerComponent, FFortClientMarkerRequest& MarkerRequest)
{
	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(MarkerComponent->GetOwner());

	if (PlayerController == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	FFortWorldMarkerData WorldMarkerData{};

	((void (*)(FFortWorldMarkerData*))(InSDKUtils::GetImageBase() + 0x128F150))(&WorldMarkerData);

	WorldMarkerData.Owner = PlayerState;
	WorldMarkerData.MarkerType = MarkerRequest.MarkerType;
	WorldMarkerData.WorldPosition = MarkerRequest.WorldPosition;
	WorldMarkerData.WorldPositionOffset = MarkerRequest.WorldPositionOffset;
	WorldMarkerData.WorldNormal = MarkerRequest.WorldNormal;
	WorldMarkerData.bHasCustomDisplayInfo = false;

	FMarkerID MarkerID{};

	MarkerID.PlayerID = PlayerState->PlayerID;
	MarkerID.InstanceID = MarkerRequest.InstanceID;

	WorldMarkerData.MarkerID = MarkerID;

	if (MarkerRequest.MarkedActor)
	{
		((char (*)(UAthenaMarkerComponent*, AActor*, FFortWorldMarkerData*))(InSDKUtils::GetImageBase() + 0x1642450))(MarkerComponent, MarkerRequest.MarkedActor, &WorldMarkerData);

		if (AFortPickup* Pickup = Cast<AFortPickup>(MarkerRequest.MarkedActor))
		{
			WorldMarkerData.ItemDefinition = Pickup->PrimaryPickupItemEntry.ItemDefinition;
			WorldMarkerData.ItemCount = Pickup->PrimaryPickupItemEntry.Count;
		}
	}

	for (auto& TeamMember_ : PlayerState->PlayerTeam->TeamMembers)
	{
		AFortPlayerControllerAthena* TeamMember = Cast<AFortPlayerControllerAthena>(TeamMember_);

		if (TeamMember_ == NULL || TeamMember == PlayerController)
			continue;

		UAthenaMarkerComponent* TeamMemberMarkerComponent = TeamMember->MarkerComponent;

		if (TeamMemberMarkerComponent == NULL)
			continue;

		TeamMemberMarkerComponent->ClientAddMarker(WorldMarkerData);
	}
}

void Misc::ServerRemoveMapMarker(UAthenaMarkerComponent* MarkerComponent, FMarkerID MarkerID, ECancelMarkerReason CancelReason)
{
	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(MarkerComponent->GetOwner());

	if (PlayerController == NULL)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (PlayerState == NULL)
		return;

	for (auto& TeamMember_ : PlayerState->PlayerTeam->TeamMembers)
	{
		AFortPlayerControllerAthena* TeamMember = Cast<AFortPlayerControllerAthena>(TeamMember_);

		if (TeamMember_ == NULL || TeamMember == PlayerController)
			continue;

		UAthenaMarkerComponent* TeamMemberMarkerComponent = TeamMember->MarkerComponent;

		if (TeamMemberMarkerComponent == NULL)
			continue;

		TeamMemberMarkerComponent->ClientCancelMarker(MarkerID);
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

	Utils::Virtual<UAthenaMarkerComponent>(0x7F, ServerAddMapMarker);
	Utils::Virtual<UAthenaMarkerComponent>(0x7D, ServerRemoveMapMarker);

	Utils::Patch(InSDKUtils::GetImageBase() + 0x19B1660, 0xC3);
	Utils::Patch(InSDKUtils::GetImageBase() + 0x27C47A0, 0xC3);
	Utils::Patch(InSDKUtils::GetImageBase() + 0x1591C25, 0x85);

	Utils::Hook(InSDKUtils::GetImageBase() + 0x372FB80, ForceRetTrue);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x27E8B00, ForceRetTrue);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x163DCA0, ForceRetTrue);

	Utils::Hook(InSDKUtils::GetImageBase() + 0xB625A0, ForceRetTrue);

	Utils::Hook(InSDKUtils::GetImageBase() + 0x3B76FE0, GetNetMode);

	Utils::Rel32(InSDKUtils::GetImageBase() + 0x3B2BF8A, Listen);
}