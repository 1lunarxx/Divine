#include "pch.h"
#include "Abilities.h"
#include "Utils.h"

void Abilities::InternalServerTryActivateAbility(UAbilitySystemComponent*, FGameplayAbilitySpecHandle, bool, FPredictionKey&, FGameplayEventData*)
{
}

void Abilities::Patch()
{
	//Utils::Virtual<UAbilitySystemComponent>(0xF7, InternalServerTryActivateAbility);
	//Utils::Virtual<UFortAbilitySystemComponent>(0xF7, InternalServerTryActivateAbility);
	//Utils::Virtual<UFortAbilitySystemComponentAthena>(0xF7, InternalServerTryActivateAbility);
}