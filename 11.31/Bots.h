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
		static inline void (*ApplyCharacterCustomization)(AFortAthenaAIBotController*, FFortAthenaLoadout*, UFortBotNameSettings*);

	};

private:
	static void InitializeMMRInfos(AFortAthenaMutator_Bots*);
	static void InitializeInventory(AFortAthenaAIBotController*, UFortAthenaAIBotInventoryItems*);

	static void OnPawnSpawned(AFortAthenaMutator_Bots*, AFortPlayerPawn*);
	static void OnPossessedPawnDied(AFortAthenaAIBotController*, AActor*, float, AFortPlayerControllerAthena*, AActor*, FVector, UPrimitiveComponent*, FName, FVector);

	static void ApplyCharacterCustomization(AFortAthenaAIBotController*, FFortAthenaLoadout*, UFortBotNameSettings*);

	static UFortWorldItem* CreateItem(AFortAthenaAIBotController*, UFortItemDefinition*, int32);

	static UFortServerBotManagerAthena* GetBotManager();
	static UObject* GetSquadLeader(AFortAthenaAIBotController*);
public:
	static void EnterAircraft(AFortAthenaAIBotController*, AFortAthenaAircraft*);
	static void Setup();
};