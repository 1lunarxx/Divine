#pragma once
#include "pch.h"

class Bots
{
	class Originals
	{
	public:
		static inline void (*InitializeMMRInfos)(AFortAthenaMutator_Bots*);
		static inline void (*OnBotSpawned)(AFortAthenaMutator_Bots*, AFortPlayerPawn*);
		static inline void (*InitializeInventory)(AFortAthenaAIBotController*, UFortAthenaAIBotInventoryItems*);
		static inline void (*GetCosmeticLoadout)(AFortAthenaAIBotController*, void*, void*);
		static inline void (*OnPossessedPawnDied)(AFortAthenaAIBotController*, AActor*, float, AFortPlayerControllerAthena*, AActor*, FVector, UPrimitiveComponent*, FName, FVector);
	};

private:
	static void InitializeMMRInfos(AFortAthenaMutator_Bots*);
	static void OnBotSpawned(AFortAthenaMutator_Bots*, AFortPlayerPawn*);

	static void InitializeInventory(AFortAthenaAIBotController*, UFortAthenaAIBotInventoryItems*);
	static void AddStartingItemsToInventory(AFortAthenaAIBotController*);

	static void GetCosmeticLoadout(AFortAthenaAIBotController*, void*, void*);
	static void OnPossessedPawnDied(AFortAthenaAIBotController*, AActor*, float, AFortPlayerControllerAthena*, AActor*, FVector, UPrimitiveComponent*, FName, FVector);

	static AFortAthenaAIBotController* GetSquadLeader(AFortAthenaAIBotController*);
	static UFortServerBotManagerAthena* GetServerBotManager();
public:
	static void EnterAircraft(AFortAthenaAIBotController*, AFortAthenaAircraft*);
	static void Setup();
};