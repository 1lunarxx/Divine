#pragma once
#include "pch.h"

class Building
{
	class Originals
	{
	public:
		static inline void (*ServerEditBuildingActor)(AFortPlayerController*, ABuildingSMActor*, TSubclassOf<ABuildingSMActor>, uint8, bool);
	};

private:
	static void ServerBeginEditingBuildingActor(AFortPlayerController*, ABuildingSMActor*);
	static void ServerEditBuildingActor(AFortPlayerController*, ABuildingSMActor*, TSubclassOf<ABuildingSMActor>, uint8, bool);
	static void ServerEndEditingBuildingActor(AFortPlayerController*, ABuildingSMActor*);
	static void ServerCreateBuildingActor(AFortPlayerControllerAthena*, FCreateBuildingActorData);
	static void AttemptSpawnResources(ABuildingSMActor*, AFortPlayerPawn*, float, bool);
public:
	static void Setup();
};