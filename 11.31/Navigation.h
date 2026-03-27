#pragma once
#include "pch.h"

class Navigation
{
	class Originals
	{
	public:
		static inline void (*InitializeForWorld)(UNavigationSystemV1*, UWorld*, EFNavigationSystemRunMode);
		static inline void (*CreateAndConfigureNavigationSystem)(UAthenaNavSystemConfig*, UWorld*);
	};

private:
	static void InitializeForWorld(UNavigationSystemV1*, UWorld*, EFNavigationSystemRunMode);
	static void CreateAndConfigureNavigationSystem(UAthenaNavSystemConfig*, UWorld*);
public:
	static void Setup();
};