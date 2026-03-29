#pragma once
#include "pch.h"

class Navigation
{
	class Originals
	{
	public:
		static inline void (*InitializeForWorld)(UNavigationSystemV1*, UWorld*, EFNavigationSystemRunMode);
	};

private:
	static void InitializeForWorld(UNavigationSystemV1*, UWorld*, EFNavigationSystemRunMode);
public:
	static void Setup();
};