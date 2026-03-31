#pragma once
#include "pch.h"

class Player
{
	class Originals
	{
	public:
		static inline void (*MovingEmoteStopped)(AFortPawn*);
		static inline void (*GetPlayerViewPoint)(AFortPlayerControllerAthena*, FVector&, FRotator&);
		static inline void (*GivePickupTo)(AFortPickup*, IFortInventoryOwnerInterface*, bool);
		static inline void (*ServerEditBuildingActor)(AFortPlayerController*, ABuildingSMActor*, TSubclassOf<ABuildingSMActor>, uint8, bool);
	};

private:
	static void ServerAcknowledgePossession(AFortPlayerControllerAthena*, APawn*);
	static void ServerPlayEmoteItem(AFortPlayerControllerAthena*, UFortItemDefinition*, float);
	static void GetPlayerViewPoint(AFortPlayerControllerAthena*, FVector&, FRotator&);

	static void ServerSendZiplineState(AFortPlayerPawn*, FZiplinePawnState&);

	static void ServerExecuteInventoryItem(AFortPlayerController*, FGuid&);
	static void ServerAttemptAircraftJump(UFortControllerComponent_Aircraft*, FRotator&);

	static void MovingEmoteStopped(AFortPawn*);
	static void ServerReviveFromDBNO(AFortPlayerPawnAthena*, AController*);

	static void ServerAddMapMarker(UAthenaMarkerComponent*, FFortClientMarkerRequest&);
	static void ServerRemoveMapMarker(UAthenaMarkerComponent*, FMarkerID, ECancelMarkerReason);

	static void ServerCheat(AFortPlayerController*, FString&);

	static void ServerAttemptInventoryDrop(AFortPlayerController*, FGuid&, int32, bool);
	static void ServerHandlePickup(AFortPlayerPawn*, AFortPickup*, float, FVector&, bool);
	static void GivePickupTo(AFortPickup*, IFortInventoryOwnerInterface*, bool);

	static void ServerBeginEditingBuildingActor(AFortPlayerController*, ABuildingSMActor*);
	static void ServerEditBuildingActor(AFortPlayerController*, ABuildingSMActor*, TSubclassOf<ABuildingSMActor>, uint8, bool);
	static void ServerEndEditingBuildingActor(AFortPlayerController*, ABuildingSMActor*);
	static void ServerCreateBuildingActor(AFortPlayerControllerAthena*, FCreateBuildingActorData);

	static void RemoveAllInventoryItems(AFortPlayerControllerAthena*);
public:
	static void Setup();
};