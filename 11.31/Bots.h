#pragma once
#include "pch.h"

class Bots
{
	class Originals
	{
	public:
		static inline void (*OnSafeZoneUpdated)(__int64);
	};
public:
	static void OnSafeZoneUpdated(__int64);
	static void Patch();
};