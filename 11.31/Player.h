#pragma once
#include "pch.h"

class Player
{
private:
	static void ServerAcknowledgePossession(AFortPlayerControllerAthena*, APawn*);
	static void RemoveAllInventoryItems(AFortPlayerControllerAthena*);
	static void ServerExecuteInventoryItem(AFortPlayerController*, FGuid&);
	static void ServerAttemptAircraftJump(UFortControllerComponent_Aircraft*, FRotator&);
public:
	static void Setup();
};