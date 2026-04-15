#pragma once
#include "pch.h"

class Bots
{
	class Originals
	{
	public:
		static inline void (*OnPawnSpawned)(AFortAthenaMutator_Bots*, AFortPlayerPawn*);
		static inline void (*InitializeInventory)(AFortAthenaAIBotController*, UFortAthenaAIBotInventoryItems*);
		static inline void (*OnPossessedPawnDied)(AFortAthenaAIBotController*, AActor*, float, AFortPlayerControllerAthena*, AActor*, FVector, UPrimitiveComponent*, FName, FVector);
		static inline void (*InitializeMMRInfos)(AFortAthenaMutator_Bots*);
		static inline UFortHero* (*GetOrInitializeHero)(AFortAthenaAIBotController*);
	};

private:
	static void InitializeMMRInfos(AFortAthenaMutator_Bots*);
	static void OnPawnSpawned(AFortAthenaMutator_Bots*, AFortPlayerPawn*);

	static void InitializeInventory(AFortAthenaAIBotController*, UFortAthenaAIBotInventoryItems*);
	static void OnPossessedPawnDied(AFortAthenaAIBotController*, AActor*, float, AFortPlayerControllerAthena*, AActor*, FVector, UPrimitiveComponent*, FName, FVector);

	static UFortWorldItem* CreateItem(AFortAthenaAIBotController*, UFortItemDefinition*, int32);

	static UFortHero* GetOrInitializeHero(AFortAthenaAIBotController*);
	static UObject* GetSquadLeader(AFortAthenaAIBotController*);
	static UFortServerBotManagerAthena* GetBotManager();
public:
	static void EnterAircraft(AFortAthenaAIBotController*, AFortAthenaAircraft*);
	static void Setup();
};