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

	static void ServerAddMapMarker(UAthenaMarkerComponent*, FFortClientMarkerRequest&);
	static void ServerRemoveMapMarker(UAthenaMarkerComponent*, FMarkerID, ECancelMarkerReason);
	static void ServerCheat(AFortPlayerController*, FString&);

	static void ServerBeginEditingBuildingActor(AFortPlayerController*, ABuildingSMActor*);
	static void ServerEditBuildingActor(AFortPlayerController*, ABuildingSMActor*, TSubclassOf<ABuildingSMActor>, uint8, bool);
	static void ServerEndEditingBuildingActor(AFortPlayerController*, ABuildingSMActor*);
	static void ServerCreateBuildingActor(AFortPlayerController*, FCreateBuildingActorData);
public:
	static void Setup();
};