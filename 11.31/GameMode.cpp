#include "pch.h"
#include "GameMode.h"
#include "Utils.h"
#include "Misc.h"

bool GameMode::ReadyToStartMatch(AFortGameModeAthena* GameMode)
{
    AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(GameMode->GameState);

    if (GameState->MapInfo == NULL)
        return false;

    if (!GameMode->bWorldIsReady)
    {
		UFortPlaylistAthena* Playlist = Utils::StaticFindObject<UFortPlaylistAthena>(TEXT("Playlist_DefaultSquad"), ANY_PACKAGE);

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
		GameState->CachedSafeZoneStartUp = Playlist->SafeZoneStartUp;

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
			GameMode->ServerBotManager = Utils::NewObject<UFortServerBotManagerAthena>(GameMode, Utils::StaticLoadObject<UBlueprintGeneratedClass>(TEXT("/Game/Athena/AI/Phoebe/BP_PhoebeManager.BP_PhoebeManager_C")));

			GameMode->ServerBotManager->CachedGameMode = GameMode;
			GameMode->ServerBotManager->CachedGameState = GameState;

			*(bool*)(__int64(GameMode->ServerBotManager) + 0x458) = true;

			GameMode->SpawningPolicyManager = Utils::SpawnActor<AFortAthenaSpawningPolicyManager>(FVector(), FRotator(), AFortAthenaSpawningPolicyManager::StaticClass(), GameMode);

			GameMode->SpawningPolicyManager->GameStateAthena = GameState;
			GameMode->SpawningPolicyManager->GameModeAthena = GameMode;

			AAthenaAIDirector* AIDirector = Utils::SpawnActor<AAthenaAIDirector>(FVector(), FRotator(), AAthenaAIDirector::StaticClass(), GameMode);

			GameMode->AIDirector = AIDirector;

			AFortAIGoalManager* AIGoalManager = Utils::SpawnActor<AFortAIGoalManager>(FVector(), FRotator(), AFortAIGoalManager::StaticClass(), GameMode);

			GameMode->AIGoalManager = AIGoalManager;

			TArray<AFortPoiVolume*> Volumes = Utils::GetAllActors<AFortPoiVolume>();

			for (AFortPoiVolume* Volume : Volumes)
			{
				FCachedPOIVolumeLocations CachedPOIVolumeLocation = FCachedPOIVolumeLocations{};

				CachedPOIVolumeLocation.POIVolume = Volume;

				FVector Origin;
				FVector BoxExtent;

				Volume->GetActorBounds(false, &Origin, &BoxExtent);

				for (int32 i = 0; i < 250; i++)
				{
					FVector RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(Origin, BoxExtent);
					CachedPOIVolumeLocation.NavMeshLocations.Add(RandomPoint);
				}

				CachedPOIVolumeLocation.Flags &= ~0x1;

				GameMode->ServerBotManager->CachedValidPOIVolumeLocations.Add(CachedPOIVolumeLocation);
			}

			for (FNavigationPOI& Tag : GameMode->ServerBotManager->OnGroundTagQueryPOIList)
			{
				for (AFortPoiVolume* Volume : Volumes)
				{
					if (Volume->ContainsLocationTag(Tag.POIFilterQuery.TagDictionary[0]))
					{
						Tag.ValidPOIVolumeList.Add(Volume);
					}
				}
			}

			for (FBattleBusPOI& Tag : GameMode->ServerBotManager->SecondaryBattleBusTagQueryPOIList)
			{
				for (AFortPoiVolume* Volume : Volumes)
				{
					if (Volume->ContainsLocationTag(Tag.POIFilterQuery.TagDictionary[0]))
					{
						Tag.ValidPOIVolumeList.Add(Volume);
					}
				}
			}

			for (FBattleBusPOI& Tag : GameMode->ServerBotManager->BattleBusTagQueryPOIList)
			{
				for (AFortPoiVolume* Volume : Volumes)
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

		SetConsoleTitleA("Divine | Joinable");

        GameMode->bWorldIsReady = true;
    }

	bool bReadyToStartMatch = GameMode->bDelayedStart == 0 && GameMode->MatchState == UKismetStringLibrary::Conv_StringToName(TEXT("WaitingToStart")) && GameMode->NumPlayers + GameMode->NumBots > 0;

	if (bReadyToStartMatch)
	{
		if (UFortServerBotManagerAthena* ServerBotManager = GameMode->ServerBotManager)
		{
			ServerBotManager->CachedBotMutator->InitializeMMRInfos();
			ServerBotManager->CachedBotMutator->StartSpawning();
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

	FGameMemberInfo GameMemberInfo = FGameMemberInfo{};

	GameMemberInfo.ReplicationID = -1;
	GameMemberInfo.ReplicationKey = -1;
	GameMemberInfo.MostRecentArrayReplicationKey = -1;

	GameMemberInfo.SquadId = PlayerState->SquadId;
	GameMemberInfo.TeamIndex = PlayerState->TeamIndex;
	GameMemberInfo.MemberUniqueId = PlayerState->UniqueId;

	GameState->GameMemberInfoArray.Members.Add(GameMemberInfo);
	GameState->GameMemberInfoArray.MarkArrayDirty();

	GameMemberInfo.PostReplicatedAdd(&GameState->GameMemberInfoArray);
}

APawn* GameMode::SpawnDefaultPawnFor(AGameModeBase* GameMode, AController* NewPlayer, AActor* StartSpot)
{
	return GameMode->SpawnDefaultPawnAtTransform(NewPlayer, StartSpot->GetTransform());
}

EFortTeam GameMode::PickTeam(AFortGameModeAthena* GameMode, uint8 PreferredTeam, AFortPlayerControllerAthena* ControllerToPickFor)
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
}