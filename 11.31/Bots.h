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
		static inline void (*BTTask_Dive_TickTask)(UFortAthenaBTTask_Dive*, UBehaviorTreeComponent*, unsigned __int8*, float);
		static inline void (*BTTask_Glide_TickTask)(UFortAthenaBTTask_Glide*, UBehaviorTreeComponent*, unsigned __int8*, double);
		static inline void (*InitializeForWorld)(UAthenaNavSystem*, UWorld*, uint8);
	};
public:
	static void BTTask_Dive_TickTask(UFortAthenaBTTask_Dive*, UBehaviorTreeComponent*, unsigned __int8*, float);
	static void BTTask_Glide_TickTask(UFortAthenaBTTask_Glide*, UBehaviorTreeComponent*, unsigned __int8*, double);

	static void CreateAndConfigureNavigationSystem(UAthenaNavSystemConfig*, UWorld*);
	static void OnSafeZoneUpdated(__int64);
	static void OnPawnSpawned(AFortAthenaMutator_Bots*, AFortPlayerPawn*);
	static void EnterAircraft(AFortAthenaAIBotController*, AFortAthenaAircraft*);
	static void InitializeForWorld(UAthenaNavSystem*, UWorld*, uint8);

	static void Tick();
	static void Patch();
};