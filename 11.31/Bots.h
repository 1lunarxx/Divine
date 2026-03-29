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
	};

private:
	static void InitializeMMRInfos(AFortAthenaMutator_Bots*);
	static void OnBotSpawned(AFortAthenaMutator_Bots*, AFortPlayerPawn*);

	static void InitializeInventory(AFortAthenaAIBotController*, UFortAthenaAIBotInventoryItems*);
	static void AddStartingItemsToInventory(AFortAthenaAIBotController*);

	static AFortAthenaAIBotController* GetSquadLeader(AFortAthenaAIBotController* BotController);

	static void GetCosmeticLoadout(AFortAthenaAIBotController*, void*, void*);
public:
	static void EnterAircraft(AFortAthenaAIBotController*, AFortAthenaAircraft*);
	static void Setup();
};