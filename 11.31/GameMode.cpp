#include "pch.h"
#include "GameMode.h"
#include "Utils.h"
#include "Misc.h"
#include "Configuration.h"

bool GameMode::ReadyToStartMatch(AFortGameModeAthena* GameMode)
{
    AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(GameMode->GameState);

    if (GameState->MapInfo == NULL)
        return false;

    if (!GameMode->bWorldIsReady)
    {
		UFortPlaylistAthena* Playlist = Utils::StaticFindObject<UFortPlaylistAthena>(TEXT("Playlist_DefaultSolo"), ANY_PACKAGE);

		GameState->CurrentPlaylistId = GameMode->CurrentPlaylistId;

		GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
		GameState->CurrentPlaylistInfo.PlaylistReplicationKey++;

		GameState->CurrentPlaylistInfo.MarkArrayDirty();

		GameState->OnRep_CurrentPlaylistId();
		GameState->OnRep_CurrentPlaylistInfo();

		GameMode->WarmupRequiredPlayerCount = 1;
		GameMode->GameSession->MaxPlayers = Playlist->MaxPlayers;

		GameMode->AISettings = Playlist->AISettings;

		GameState->AirCraftBehavior = Playlist->AirCraftBehavior;

		GameState->Client_RefreshEventCalendar();

		for (TSoftObjectPtr<UWorld>& AdditionalLevel : Playlist->AdditionalLevels)
		{
			bool bSuccess = false;
			ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(UWorld::GetWorld(), AdditionalLevel, FVector(), FRotator(), &bSuccess);

			if (bSuccess)
				GameState->AdditionalPlaylistLevelsStreamed.Add(AdditionalLevel.ObjectID.AssetPathName);
		}

		GameState->OnRep_AdditionalPlaylistLevelsStreamed();
		GameState->OnFinishedStreamingAdditionalPlaylistLevel();

		if (GameMode->ServerBotManager == NULL)
		{
			UFortServerBotManagerAthena* ServerBotManager = Utils::NewObject<UFortServerBotManagerAthena>(GameMode, Utils::StaticLoadObject<UBlueprintGeneratedClass>(TEXT("/Game/Athena/AI/Phoebe/BP_PhoebeManager.BP_PhoebeManager_C")));

			GameMode->ServerBotManager = ServerBotManager;
			GameMode->ServerBotManager->CachedGameMode = GameMode;
			GameMode->ServerBotManager->CachedGameState = GameState;

			*(bool*)(__int64(GameMode->ServerBotManager) + 0x4CE) = true;

			AFortAthenaSpawningPolicyManager* SpawningPolicyManager = Utils::SpawnActor<AFortAthenaSpawningPolicyManager>(FVector(), FRotator(), AFortAthenaSpawningPolicyManager::StaticClass(), GameMode);

			GameMode->SpawningPolicyManager = SpawningPolicyManager;
			GameMode->SpawningPolicyManager->GameStateAthena = GameState;
			GameMode->SpawningPolicyManager->GameModeAthena = GameMode;

			AAthenaAIDirector* AIDirector = Utils::SpawnActor<AAthenaAIDirector>(FVector(), FRotator(), AAthenaAIDirector::StaticClass(), GameMode);

			GameMode->AIDirector = AIDirector;

			AFortAIGoalManager* AIGoalManager = Utils::SpawnActor<AFortAIGoalManager>(FVector(), FRotator(), AFortAIGoalManager::StaticClass(), GameMode);

			GameMode->AIGoalManager = AIGoalManager;

			TArray<AFortPoiVolume*> Volumes = Utils::GetAllActors<AFortPoiVolume>();

			for (auto& Volume : Volumes)
			{
				FCachedPOIVolumeLocations VolumeLocations{};

				VolumeLocations.POIVolume = Volume;

				FVector Origin;
				FVector BoxExtent;

				Volume->GetActorBounds(false, &Origin, &BoxExtent);

				for (int i = 0; i < 250; i++)
				{
					FVector RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(Origin, BoxExtent);
					VolumeLocations.NavMeshLocations.Add(RandomPoint);
				}

				VolumeLocations.Flags &= ~0x1;
				ServerBotManager->CachedValidPOIVolumeLocations.Add(VolumeLocations);
			}

			for (FNavigationPOI& Tag : ServerBotManager->OnGroundTagQueryPOIList)
			{
				for (auto& Volume : Volumes)
				{
					if (Volume->ContainsLocationTag(Tag.POIFilterQuery.TagDictionary[0]))
					{
						Tag.ValidPOIVolumeList.Add(Volume);
					}
				}
			}

			for (FBattleBusPOI& Tag : ServerBotManager->SecondaryBattleBusTagQueryPOIList)
			{
				for (auto& Volume : Volumes)
				{
					if (Volume->ContainsLocationTag(Tag.POIFilterQuery.TagDictionary[0]))
					{
						Tag.ValidPOIVolumeList.Add(Volume);
					}
				}
			}

			for (FBattleBusPOI& Tag : ServerBotManager->BattleBusTagQueryPOIList)
			{
				for (auto& Volume : Volumes)
				{
					if (Volume->ContainsLocationTag(Tag.POIFilterQuery.TagDictionary[0]))
					{
						Tag.ValidPOIVolumeList.Add(Volume);
					}
				}
			}
		}

		for (AFortAthenaVehicleSpawner* VehicleSpawner : Utils::GetAllActors<AFortAthenaVehicleSpawner>())
		{
			if (VehicleSpawner->GetVehicleClass() != NULL)
			{
				Utils::SpawnActor<AFortAthenaVehicle>(VehicleSpawner->K2_GetActorLocation(), VehicleSpawner->K2_GetActorRotation(), VehicleSpawner->GetVehicleClass());
			}
		}

		if (UAthenaBattleBusItemDefinition* BBID_WinterBus = Utils::StaticLoadObject<UAthenaBattleBusItemDefinition>(TEXT("/Game/Athena/Items/Cosmetics/BattleBuses/BBID_WinterBus.BBID_WinterBus")))
		{
			if (UFortAssetManager* AssetManager = Cast<UFortAssetManager>(UFortEngine::GetEngine()->AssetManager))
			{
				AssetManager->GameDataCosmetics->DefaultBattleBusSkin = BBID_WinterBus;
			}
		}

		SetConsoleTitleA("Divine | Joinable");

        GameMode->bWorldIsReady = true;
    }

	bool bReadyToStartMatch = GameMode->bDelayedStart == 0 && GameMode->MatchState == UKismetStringLibrary::Conv_StringToName(TEXT("WaitingToStart")) && GameMode->NumPlayers + GameMode->NumBots > 0;

	if (bReadyToStartMatch)
	{
		if (UFortServerBotManagerAthena* ServerBotManager = GameMode->ServerBotManager)
		{
			static void (*InitializeMMRInfos)(AFortAthenaMutator_Bots*) = decltype(InitializeMMRInfos)(InSDKUtils::GetImageBase() + 0x1654BB0);
			InitializeMMRInfos(ServerBotManager->CachedBotMutator);

			static void (*StartSpawning)(AFortAthenaMutator_Bots*, __int64, char) = decltype(StartSpawning)(InSDKUtils::GetImageBase() + 0x1665B00);
			StartSpawning(ServerBotManager->CachedBotMutator, 0, 0);
		}
	}

	return bReadyToStartMatch;
}

void GameMode::HandleStartingNewPlayer(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	Originals::HandleStartingNewPlayer(GameMode, NewPlayer);

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(NewPlayer->PlayerState);

	if (PlayerState == NULL)
		return;

	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(GameMode->GameState);

	if (GameState == NULL)
		return;

	PlayerState->SquadId = PlayerState->TeamIndex - 3;
	PlayerState->OnRep_SquadId();

	PlayerState->WorldPlayerId = PlayerState->PlayerID;

	FGameMemberInfo MemberInfo = FGameMemberInfo{};

	MemberInfo.ReplicationID = -1;
	MemberInfo.ReplicationKey = -1;
	MemberInfo.MostRecentArrayReplicationKey = -1;

	MemberInfo.SquadId = PlayerState->SquadId;
	MemberInfo.TeamIndex = PlayerState->TeamIndex;
	MemberInfo.MemberUniqueId = PlayerState->UniqueId;

	GameState->GameMemberInfoArray.Members.Add(MemberInfo);
	GameState->GameMemberInfoArray.MarkArrayDirty();

	GameState->NotifyGameMemberAdded(PlayerState->SquadId, PlayerState->TeamIndex, PlayerState->UniqueId);
}

APawn* GameMode::SpawnDefaultPawnFor(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* NewPlayer, AActor* StartSpot)
{
    return GameMode->SpawnDefaultPawnAtTransform(NewPlayer, StartSpot->GetTransform());
}

EFortTeam GameMode::PickTeam(AFortGameModeAthena* GameMode, uint8_t PreferredTeam, AFortPlayerControllerAthena* Controller) 
{
	static uint8_t Team = 3;
	static uint8_t PlayersOnTeam = 0;

	uint8_t ChosenTeam = Team;

	if (AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(GameMode->GameState))
	{
		UFortPlaylistAthena* Playlist = GameState->CurrentPlaylistInfo.BasePlaylist;

		if (Playlist == NULL)
			return EFortTeam::MAX;

		if (++PlayersOnTeam >= Playlist->MaxSquadSize)
		{
			Team++;
			PlayersOnTeam = 0;
		}
	}

	return EFortTeam(ChosenTeam);
}

void GameMode::Setup()
{
    Utils::Virtual<AFortGameModeAthena>(0xC5, SpawnDefaultPawnFor);
    Utils::Virtual<AFortGameModeAthena>(0xFE, ReadyToStartMatch);
	Utils::Virtual<AFortGameModeAthena>(0xCB, HandleStartingNewPlayer, (void**)&Originals::HandleStartingNewPlayer);

	Utils::Hook(InSDKUtils::GetImageBase() + 0x153B210, PickTeam);

	if (Configuration::bUseGameSessions)
	{
		//Utils::Hook(InSDKUtils::GetImageBase() + 0x199DAA0, GetGameSessionClass);
	}
}