#pragma once
#include "pch.h"

class GameMode
{
	class Originals
	{
	public:
		static inline void (*StartNewSafeZonePhase)(AFortGameModeAthena*, int);
		static inline void (*HandleStartingNewPlayer)(AGameModeBase*, APlayerController*);
	};

private:
	static bool ReadyToStartMatch(AFortGameModeAthena*);
	static void StartNewSafeZonePhase(AFortGameModeAthena*, int);
	static void HandleStartingNewPlayer(AGameModeBase*, APlayerController*);

	static APawn* SpawnDefaultPawnFor(AFortGameModeAthena*, AFortPlayerControllerAthena*, AActor*);
	static EFortTeam PickTeam(AFortGameModeAthena*, uint8_t, AFortPlayerControllerAthena*);
public:
	static void Setup();
};