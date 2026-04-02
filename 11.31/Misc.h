#pragma once
#include "pch.h"

class Misc
{
public:
	class Originals
	{
	public:
		static inline void (*TickFlush)(UNetDriver*);
		static inline void (*SendRequestNow)(UMcpProfileGroup*, const void*, FBaseUrlContext::EContextCredentials);
		static inline void (*PlacePlayersInAircraft)(AFortGameStateAthena*);
		static inline void (*PostInitializeComponents)(AFortPoiVolume*);
	};

private:
	static void TickFlush(UNetDriver*);
	static void SendRequestNow(UMcpProfileGroup*, const void*, FBaseUrlContext::EContextCredentials);
	static void PlacePlayersInAircraft(AFortGameStateAthena*);

	static float GetMaxTickRate(UEngine*, float, bool);
	static bool Listen();
public:
	static void Setup();
};