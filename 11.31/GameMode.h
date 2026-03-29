#pragma once
#include "pch.h"

class GameMode
{
	class Originals
	{
	public:
		static inline void (*PlaceBotOnTeam)(AFortGameModeAthena*, AFortPlayerStateAthena*, AFortTeamInfoAthena*, uint8);
		static inline void (*HandleStartingNewPlayer)(AGameModeBase*, APlayerController*);
	};

private:
	static bool ReadyToStartMatch(AFortGameModeAthena*);
	static void PlaceBotOnTeam(AFortGameModeAthena*, AFortPlayerStateAthena*, AFortTeamInfoAthena*, uint8);
	static void HandleStartingNewPlayer(AGameModeBase*, APlayerController*);

	static APawn* SpawnDefaultPawnFor(AFortGameModeAthena*, AFortPlayerControllerAthena*, AActor*);
	static EFortTeam PickTeam(AFortGameModeAthena* GameMode, uint8_t PreferredTeam, AFortPlayerControllerAthena* Controller);
public:
	static void Setup();
};