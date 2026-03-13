#include "pch.h"
#include "Abilities.h"
#include "Utils.h"

void Abilities::InternalServerTryActivateAbility(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, FPredictionKey& PredictionKey, FGameplayEventData* TriggerEventData)
{
	FGameplayAbilitySpec* Spec = nullptr;

	for (auto& Item : AbilitySystemComponent->ActivatableAbilities.Items)
	{
		if (Item.Handle.Handle == Handle.Handle)
		{
			Spec = &Item;
			break;
		}
	}

	if (Spec == NULL)
	{
		return AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
	}

	UGameplayAbility* AbilityToActivate = Spec->Ability;
	if (AbilityToActivate == NULL)
	{
		return AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
	}

	UGameplayAbility* InstancedAbility = nullptr;
	Spec->InputPressed = true;

	auto InternalTryActivateAbility = (bool (*)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle, FPredictionKey, UGameplayAbility**, void*, const FGameplayEventData*)) (InSDKUtils::GetImageBase() + 0xB78580);
	if (!InternalTryActivateAbility(AbilitySystemComponent, Handle, PredictionKey, &InstancedAbility, nullptr, TriggerEventData))
	{
		AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
		Spec->InputPressed = false;
	}

	AbilitySystemComponent->ActivatableAbilities.MarkItemDirty(*Spec);
}

void Abilities::Patch()
{
	Utils::Virtual<UAbilitySystemComponent>(0xF7, InternalServerTryActivateAbility);
	Utils::Virtual<UFortAbilitySystemComponent>(0xF7, InternalServerTryActivateAbility);
	Utils::Virtual<UFortAbilitySystemComponentAthena>(0xF7, InternalServerTryActivateAbility);
}