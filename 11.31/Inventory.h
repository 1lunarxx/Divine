#pragma once
#include "pch.h"

class Inventory
{
public:
	static UFortWorldItem* CreateItem(AFortInventory*, UFortItemDefinition*, int32 = 1, int32 = 0);

	static FFortItemEntry* GetItemEntry(AFortInventory*, FGuid&);
};