#pragma once
#include "pch.h"

class Bots
{
	class Originals
	{
	public:
		static inline void (*InitializeMMRInfos)(AFortAthenaMutator_Bots*);
		static inline void (*OnBotSpawned)(AFortAthenaMutator_Bots*, AFortPlayerPawn*);
		static inline void (*InitializeForWorld)(UNavigationSystemV1*, UWorld*, EFNavigationSystemRunMode);
		static inline void (*CreateAndConfigureNavigationSystem)(UAthenaNavSystemConfig*, UWorld*);
		static inline void (*InitializeInventory)(AFortAthenaAIBotController*, UFortAthenaAIBotInventoryItems*);
		static inline void (*GetCosmeticLoadout)(AFortAthenaAIBotController*, __int64, __int64);
	};

private:
	static void InitializeMMRInfos(AFortAthenaMutator_Bots*);
	static void OnBotSpawned(AFortAthenaMutator_Bots*, AFortPlayerPawn*);

	static void InitializeForWorld(UNavigationSystemV1*, UWorld*, EFNavigationSystemRunMode);
	static void CreateAndConfigureNavigationSystem(UAthenaNavSystemConfig*, UWorld*);

	static void InitializeInventory(AFortAthenaAIBotController*, UFortAthenaAIBotInventoryItems*);
	static void AddStartingItemsToInventory(AFortAthenaAIBotController*);

	static AFortAthenaAIBotController* GetSquadLeader(AFortAthenaAIBotController*);

	static void GetCosmeticLoadout(AFortAthenaAIBotController*, __int64, __int64);
public:
	static void EnterAircraft(AFortAthenaAIBotController*, AFortAthenaAircraft*);
	static void Setup();
};