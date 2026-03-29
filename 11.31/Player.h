#pragma once
#include "pch.h"

class Player
{
	class Originals
	{
	public:
		static inline void (*MovingEmoteStopped)(AFortPawn*);
	};
private:
	static void ServerAcknowledgePossession(AFortPlayerControllerAthena*, APawn*);
	static void ServerPlayEmoteItem(AFortPlayerControllerAthena*, UFortItemDefinition*, float);
	static void ServerSendZiplineState(AFortPlayerPawn*, FZiplinePawnState&);
	static void RemoveAllInventoryItems(AFortPlayerControllerAthena*);
	static void ServerExecuteInventoryItem(AFortPlayerController*, FGuid&);
	static void ServerAttemptAircraftJump(UFortControllerComponent_Aircraft*, FRotator&);
	static void MovingEmoteStopped(AFortPawn*);
	static void ServerReviveFromDBNO(AFortPlayerPawnAthena*, AController*);
public:
	static void Setup();
};