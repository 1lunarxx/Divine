#pragma once
#include "pch.h"

struct FServicePermissionsMcp
{
public:
	class FString Id;
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
	static void ServerMove(AFortPhysicsPawn*, FReplicatedPhysicsPawnState&);

	static FServicePermissionsMcp* MatchMakingServicePerms(int64, int64);

	static bool Listen();
public:
	static void Setup();
};