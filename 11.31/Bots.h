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
		static inline void (*InitializeForWorld)(UAthenaNavSystem*, UWorld*, uint8);
	};
public:
	static void CreateAndConfigureNavigationSystem(UAthenaNavSystemConfig*, UWorld*);
	static void OnSafeZoneUpdated(__int64);
	static void BallsInTheFan(AFortAthenaAIBotController*);
	static void OnPawnSpawned(AFortAthenaMutator_Bots*, AFortPlayerPawn*);
	static void InitializeForWorld(UAthenaNavSystem*, UWorld*, uint8);
	static void EnterAircraft(AFortAthenaAIBotController*);
	static void Tick();

	static void Patch();
};