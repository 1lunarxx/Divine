#pragma once
#include "pch.h"

class Misc
{
public:
	class Originals
	{
	public:
		static inline void(*TickFlush)(UNetDriver*);
		static inline void (*SendRequestNow)(UMcpProfileGroup*, const void*, FBaseUrlContext::EContextCredentials);
		static inline void (*PlacePlayersInAircraft)(AFortGameStateAthena*, bool);

	};

	static int PatchRetOne()
	{
		return 1;
	}

	static void TickFlush(UNetDriver*);
	static void SendRequestNow(UMcpProfileGroup*, const void*, FBaseUrlContext::EContextCredentials);
	static float GetMaxTickRate();
	static bool Listen(UWorld*, FURL*);
	static void OverrideLootLevel(AFortGameStateAthena*, int*);
	static void PlacePlayersInAircraft(AFortGameStateAthena*, bool);

	static void Patch();
};