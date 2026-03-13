#pragma once
#include "pch.h"

class GameMode
{
public:
	static bool ReadyToStartMatch(AFortGameModeAthena*);
	static APawn* SpawnDefaultPawnFor(AFortGameModeAthena*, AFortPlayerControllerAthena*, AActor*);

	static void Patch();
};