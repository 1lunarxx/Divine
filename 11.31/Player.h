#pragma once
#include "pch.h"

class Player
{
	class Originals
	{
	public:
		static inline void (*EndSkydiving)(AFortPlayerPawn*);
	};

private:
	static void ServerAcknowledgePossession(AFortPlayerControllerAthena*, APawn*);
	static void ServerSendZiplineState(AFortPlayerPawn*, FZiplinePawnState&);
	static void RemoveAllInventoryItems(AFortPlayerControllerAthena*);
	static void ServerExecuteInventoryItem(AFortPlayerController*, FGuid&);
	static void ServerAttemptAircraftJump(UFortControllerComponent_Aircraft*, FRotator&);
	static void EndSkydiving(AFortPlayerPawn*);
public:
	static void Setup();
};