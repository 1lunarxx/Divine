#pragma once
#include "pch.h"

class Abilities
{
public:
	static void InternalServerTryActivateAbility(UAbilitySystemComponent*, FGameplayAbilitySpecHandle, bool, FPredictionKey&, FGameplayEventData*);
	static void Patch();
};