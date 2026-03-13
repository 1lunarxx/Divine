#include "pch.h"
#include "Misc.h"
#include "Utils.h"

void Misc::TickFlush(UNetDriver* NetDriver)
{
	if (NetDriver->ReplicationDriver)
	{
		static void(*ServerReplicateActors)(UReplicationDriver*) = decltype(ServerReplicateActors)(InSDKUtils::GetImageBase() + 0xC7CC50);
		ServerReplicateActors(NetDriver->ReplicationDriver);
	}

	return Originals::TickFlush(NetDriver);
}

void Misc::SendRequestNow(UMcpProfileGroup* McpProfileGroup, const void* HttpRequest, FBaseUrlContext::EContextCredentials ContextCredentials)
{
	return Originals::SendRequestNow(McpProfileGroup, HttpRequest, FBaseUrlContext::EContextCredentials::CXC_Public);
}

float Misc::GetMaxTickRate()
{
	return 30.0f;
}

bool Misc::Listen(UWorld* World, FURL* InURL)
{
	InURL->Port = 7777;

	FName NetDriverDefinition = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver");
	UNetDriver* NetDriver = ((UNetDriver * (*)(UEngine*, UWorld*, FName))(InSDKUtils::GetImageBase() + 0x3B21C20))(UEngine::GetEngine(), World, NetDriverDefinition);

	NetDriver->World = World;
	NetDriver->NetDriverName = NetDriverDefinition;

	World->NetDriver = NetDriver;

	for (FLevelCollection& LevelCollection : World->LevelCollections)
		LevelCollection.NetDriver = World->NetDriver;

	auto InitListen = reinterpret_cast<bool (*)(UNetDriver*, UWorld*, FURL&, bool, FString&)>(InSDKUtils::GetImageBase() + 0x8EFBA0);
	auto SetWorld = reinterpret_cast<void (*)(UNetDriver*, UWorld*)>(InSDKUtils::GetImageBase() + 0x3882F50);

	FString Error;

	if (!InitListen(World->NetDriver, World, *InURL, false, Error))
	{
		return false;
	}
	else
	{
		SetWorld(World->NetDriver, World);
	}

	return true;
}

void Misc::OverrideLootLevel(AFortGameStateAthena* GameState, int* OutLootLevel)
{
	AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);

	if (GameMode->CurrentPlaylistId == -1)
	{
		UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo");

		GameMode->SetCurrentPlaylistId(Playlist->PlaylistId);
		GameMode->SetCurrentPlaylistName(Playlist->PlaylistName);

		GameState->CurrentPlaylistId = GameMode->CurrentPlaylistId;

		GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
		GameState->CurrentPlaylistInfo.BasePlaylist->GarbageCollectionFrequency = 9999999999999999.f;
		GameState->CurrentPlaylistInfo.PlaylistReplicationKey++;
		GameState->CurrentPlaylistInfo.MarkArrayDirty();

		GameState->OnRep_CurrentPlaylistId();
		GameState->OnRep_CurrentPlaylistInfo();

		GameMode->WarmupRequiredPlayerCount = 1;
		GameMode->GameSession->MaxPlayers = Playlist->MaxPlayers;

		for (TSoftObjectPtr<UWorld>& AdditionalLevel : Playlist->AdditionalLevels)
		{
			bool bSuccess = false;
			ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(UWorld::GetWorld(), AdditionalLevel, FVector(), FRotator(), &bSuccess);

			if (bSuccess)
				GameState->AdditionalPlaylistLevelsStreamed.Add(AdditionalLevel.ObjectID.AssetPathName);

			GameState->OnRep_AdditionalPlaylistLevelsStreamed();
		}

		if (GameMode->ServerBotManager == NULL)
		{
			UBlueprintGeneratedClass* BP_PhoebeManager = Utils::StaticLoadObject<UBlueprintGeneratedClass>("/Game/Athena/AI/Phoebe/BP_PhoebeManager.BP_PhoebeManager_C");
			UFortServerBotManagerAthena* BotManager = (UFortServerBotManagerAthena*)UGameplayStatics::SpawnObject(BP_PhoebeManager, GameMode);

			GameMode->ServerBotManager = BotManager;
			GameMode->ServerBotManager->CachedGameMode = GameMode;
			GameMode->ServerBotManager->CachedGameState = GameState;

			*(bool*)(__int64(GameMode->ServerBotManager) + 0x458) = true;

			AFortAthenaSpawningPolicyManager* SpawningPolicyManager = Utils::SpawnActor<AFortAthenaSpawningPolicyManager>(FVector(), FRotator(), AFortAthenaSpawningPolicyManager::StaticClass(), GameMode);

			GameMode->SpawningPolicyManager = SpawningPolicyManager;
			GameMode->SpawningPolicyManager->GameStateAthena = GameState;
			GameMode->SpawningPolicyManager->GameModeAthena = GameMode;

			AAthenaAIDirector* Director = Utils::SpawnActor<AAthenaAIDirector>(FVector(), FRotator(), AAthenaAIDirector::StaticClass(), GameMode);
			GameMode->AIDirector = Director;

			AFortAIGoalManager* GoalManager = Utils::SpawnActor<AFortAIGoalManager>(FVector(), FRotator(), AFortAIGoalManager::StaticClass(), GameMode);
			GameMode->AIGoalManager = GoalManager;
		}
	}

	if (GameState->CurrentPlaylistInfo.BasePlaylist)
	{
		*OutLootLevel = GameState->CurrentPlaylistInfo.BasePlaylist->LootLevel;
	}
	else
	{
		*OutLootLevel = 1;
	}
}

void Misc::Patch()
{
	Utils::Hook(InSDKUtils::GetImageBase() + 0x3883CD0, TickFlush, (void**)&Originals::TickFlush);
	Utils::Hook(InSDKUtils::GetImageBase() + 0xE2BF70, SendRequestNow, (void**)&Originals::SendRequestNow);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x3B24510, GetMaxTickRate);
	Utils::Hook(InSDKUtils::GetImageBase() + 0x1589100, OverrideLootLevel);

	std::vector<uint64_t> NullFuncs = { InSDKUtils::GetImageBase() + 0x32eaad0, InSDKUtils::GetImageBase() + 0x19b1660, InSDKUtils::GetImageBase() + 0x27c47a0 };
	std::vector<uint64_t> RetTrueFuncs = { InSDKUtils::GetImageBase() + 0x372fb80, InSDKUtils::GetImageBase() + 0x3b76fe0 };

	for (uint64_t NullFunc : NullFuncs)
	{
		Utils::Patch(NullFunc, 0xC3);
	}

	for (uint64_t RetTrue : RetTrueFuncs)
	{
		Utils::Hook(RetTrue, PatchRetOne);
	}
}