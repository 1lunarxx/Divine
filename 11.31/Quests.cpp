#include "pch.h"
#include "Quests.h"
#include "Utils.h"

void Quests::SendStatEventWithTags()
{
	printf(__FUNCTION__);
}

void Quests::Setup()
{
	Utils::Hook(InSDKUtils::GetImageBase() + 0x1EE65B0, SendStatEventWithTags);
}