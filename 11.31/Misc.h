#pragma once
#include "pch.h"

enum ENetMode
{
	NM_Standalone,
	NM_DedicatedServer,
	NM_ListenServer,
	NM_Client,
	NM_MAX,
};

class Misc
{
public:
	class Originals
	{
	public:
		static inline void (*TickFlush)(UNetDriver*);
		static inline void (*SendRequestNow)(UMcpProfileGroup*, const void*, FBaseUrlContext::EContextCredentials);
		static inline void (*PlacePlayersInAircraft)(AFortGameStateAthena*, bool);
		static inline void (*PostInitializeComponents)(AFortPoiVolume*);
	};

private:
	static void TickFlush(UNetDriver*);
	static void SendRequestNow(UMcpProfileGroup*, const void*, FBaseUrlContext::EContextCredentials);
	static void PlacePlayersInAircraft(AFortGameStateAthena*, bool);

	static void ServerAddMapMarker(UAthenaMarkerComponent*, FFortClientMarkerRequest&);
	static void ServerRemoveMapMarker(UAthenaMarkerComponent*, FMarkerID, ECancelMarkerReason);

	static bool Listen();
private:
	static ENetMode GetNetMode()
	{
		return NM_DedicatedServer;
	}
public:
	static void Setup();
};