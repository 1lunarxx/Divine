#include "pch.h"
#include "Bots.h"
#include "Utils.h"

void Bots::OnSafeZoneUpdated(__int64 a1)
{
	*(int*)(a1 + 0x430) = 1;
	return Originals::OnSafeZoneUpdated(a1);
}

void Bots::Patch()
{
	Utils::Hook(InSDKUtils::GetImageBase() + 0x16414A0, OnSafeZoneUpdated, (void**)&Originals::OnSafeZoneUpdated);
}