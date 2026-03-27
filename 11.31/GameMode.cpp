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
		UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo");

		GameState->CurrentPlaylistId = GameMode->CurrentPlaylistId;

		GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
		GameState->CurrentPlaylistInfo.PlaylistReplicationKey++;
		GameState->CurrentPlaylistInfo.MarkArrayDirty();

		GameState->OnRep_CurrentPlaylistId();
		GameState->OnRep_CurrentPlaylistInfo();

		GameMode->WarmupRequiredPlayerCount = 1;
		GameMode->GameSession->MaxPlayers = Playlist->MaxPlayers;

		GameMode->bDBNOEnabled = Playlist->MaxSquadSize > 1;
		GameMode->AISettings = Playlist->AISettings;

		GameState->Client_RefreshEventCalendar();

		for (TSoftObjectPtr<UWorld>& AdditionalLevel : Playlist->AdditionalLevels)
		{
			bool bSuccess = false;
			ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(UWorld::GetWorld(), AdditionalLevel, FVector(), FRotator(), &bSuccess);

			GameState->AdditionalPlaylistLevelsStreamed.Add(AdditionalLevel.ObjectID.AssetPathName);
		}

		GameState->OnRep_AdditionalPlaylistLevelsStreamed();
		GameState->OnFinishedStreamingAdditionalPlaylistLevel();

		if (GameMode->ServerBotManager == NULL)
		{
			UFortServerBotManagerAthena* ServerBotManager = Utils::NewObject<UFortServerBotManagerAthena>(GameMode, Utils::StaticFindObject<UBlueprintGeneratedClass>("/Game/Athena/AI/Phoebe/BP_PhoebeManager.BP_PhoebeManager_C"));

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
			GameMode->AIDirector->Activate();

			AFortAIGoalManager* AIGoalManager = Utils::SpawnActor<AFortAIGoalManager>(FVector(), FRotator(), AFortAIGoalManager::StaticClass(), GameMode);

			GameMode->AIGoalManager = AIGoalManager;
		}

		static UAthenaBattleBusItemDefinition* BBID_WinterBus = Utils::StaticLoadObject<UAthenaBattleBusItemDefinition>("/Game/Athena/Items/Cosmetics/BattleBuses/BBID_WinterBus.BBID_WinterBus");

		if (BBID_WinterBus)
		{
			if (UFortAssetManager* AssetManager = Cast<UFortAssetManager>(UFortEngine::GetEngine()->AssetManager))
			{
				AssetManager->GameDataCosmetics->DefaultBattleBusSkin = BBID_WinterBus;
			}
		}

		SetConsoleTitleA("Winter | Joinable");

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

void GameMode::PlaceBotOnTeam(AFortGameModeAthena* GameMode, AFortPlayerStateAthena* FPSA, AFortTeamInfoAthena* TeamToBePutOn, uint8 BotSquadId)
{
	Originals::PlaceBotOnTeam(GameMode, FPSA, TeamToBePutOn, BotSquadId);

	static __int64(*TPlayerList_Find)(__int64, uint8) = decltype(TPlayerList_Find)(InSDKUtils::GetImageBase() + 0x15729D0);
	static void(*TPlayerList_Add)(__int64, AFortPlayerStateAthena*) = decltype(TPlayerList_Add)(InSDKUtils::GetImageBase() + 0x156ABD0);

	__int64 TeamIdxPlayerList = TPlayerList_Find((int64(GameMode->GameState) + 0x14F0), FPSA->TeamIndex);
	TPlayerList_Add(TeamIdxPlayerList, FPSA);

	__int64 SquadIdPlayerList = TPlayerList_Find((int64(GameMode->GameState) + 0x1528), FPSA->SquadId);
	TPlayerList_Add(SquadIdPlayerList, FPSA);
}

void GameMode::HandleStartingNewPlayer(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(NewPlayer->PlayerState);

	if (PlayerState == NULL)
		return Originals::HandleStartingNewPlayer(GameMode, NewPlayer);

	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(GameMode->GameState);

	if (GameState == NULL)
		return Originals::HandleStartingNewPlayer(GameMode, NewPlayer);

	PlayerState->SquadId = PlayerState->TeamIndex - 3;
	PlayerState->OnRep_SquadId();

	FGameMemberInfo MemberInfo{};

	MemberInfo.ReplicationID = -1;
	MemberInfo.ReplicationKey = -1;
	MemberInfo.MostRecentArrayReplicationKey = -1;

	MemberInfo.SquadId = PlayerState->SquadId;
	MemberInfo.TeamIndex = PlayerState->TeamIndex;
	MemberInfo.MemberUniqueId = PlayerState->UniqueId;

	GameState->GameMemberInfoArray.Members.Add(MemberInfo);
	GameState->GameMemberInfoArray.MarkArrayDirty();

	return Originals::HandleStartingNewPlayer(GameMode, NewPlayer);
}

APawn* GameMode::SpawnDefaultPawnFor(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* NewPlayer, AActor* StartSpot) {
    return GameMode->SpawnDefaultPawnAtTransform(NewPlayer, StartSpot->GetTransform());
}

void GameMode::Setup()
{
	//Utils::Hook(InSDKUtils::GetImageBase() + 0x153B740, PlaceBotOnTeam, (void**)&Originals::PlaceBotOnTeam);

    Utils::Virtual<AFortGameModeAthena>(0xC5, SpawnDefaultPawnFor);
    Utils::Virtual<AFortGameModeAthena>(0xFE, ReadyToStartMatch);
	Utils::Virtual<AFortGameModeAthena>(0xCB, HandleStartingNewPlayer, (void**)&Originals::HandleStartingNewPlayer);
}