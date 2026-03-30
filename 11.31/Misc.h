#pragma once
#include "pch.h"

struct FServicePermissionsMcp {
public:
	char Unk_0[0x10];
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
		static inline wchar_t* (*GetCommandLet)();

	};

private:
	static void TickFlush(UNetDriver*);
	static void SendRequestNow(UMcpProfileGroup*, const void*, FBaseUrlContext::EContextCredentials);
	static void PlacePlayersInAircraft(AFortGameStateAthena*, bool);

	static FServicePermissionsMcp* MatchMakingServicePerms(int64, int64);
	static const wchar_t* GetCommandLet();

	static bool Listen();
public:
	static void Setup();
};