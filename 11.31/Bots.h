#pragma once
#include "pch.h"

class Bots
{
	class Originals
	{
	public:
		static inline void (*OnSafeZoneUpdated)(__int64);
		static inline void (*CreateAndConfigureNavigationSystem)(UAthenaNavSystemConfig*, UWorld*);
		static inline void (*OnPawnSpawned)(AFortAthenaMutator_Bots*, AFortPlayerPawn*);
	};
public:
	static void CreateAndConfigureNavigationSystem(UAthenaNavSystemConfig*, UWorld*);
	static void OnSafeZoneUpdated(__int64);
	static void OnPawnSpawned(AFortAthenaMutator_Bots*, AFortPlayerPawn*);
	static void EnterAircraft(AFortAthenaAIBotController*, AFortAthenaAircraft*);

	static void Patch();
};