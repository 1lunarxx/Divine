#pragma once
#include "pch.h"

class Player
{
public:
	static void ServerAcknowledgePossession(AFortPlayerControllerAthena*, APawn*);
	static void ServerExecuteInventoryItem(AFortPlayerController*, FGuid&);
	static void ServerAttemptAircraftJump(UFortControllerComponent_Aircraft*, FRotator&);

	static void Patch();
};