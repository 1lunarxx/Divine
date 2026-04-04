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
		static inline void (*OnPossessedPawnDied)(AFortAthenaAIBotController*, AActor*, float, AFortPlayerControllerAthena*, AActor*, FVector, UPrimitiveComponent*, FName, FVector);
	};

private:
	static void InitializeMMRInfos(AFortAthenaMutator_Bots*);
	static void OnBotSpawned(AFortAthenaMutator_Bots*, AFortPlayerPawn*);

	static void InitializeInventory(AFortAthenaAIBotController*, UFortAthenaAIBotInventoryItems*);
	static void AddStartingItemsToInventory(AFortAthenaAIBotController*);

	static void OnPossessedPawnDied(AFortAthenaAIBotController*, AActor*, float, AFortPlayerControllerAthena*, AActor*, FVector, UPrimitiveComponent*, FName, FVector);
	static void SetCharacterCustomizationFromPlayerPawn();

	static UFortWorldItem* CreateItem(AFortAthenaAIBotController*, UFortItemDefinition*, int32);

	static AFortAthenaAIBotController* GetSquadLeader(AFortAthenaAIBotController*);
	static UFortServerBotManagerAthena* GetServerBotManager();
public:
	static void EnterAircraft(AFortAthenaAIBotController*, AFortAthenaAircraft*);
	static void Setup();
};