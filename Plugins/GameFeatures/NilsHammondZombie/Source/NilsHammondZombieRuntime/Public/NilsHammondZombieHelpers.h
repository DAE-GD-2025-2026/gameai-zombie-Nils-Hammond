#pragma once

#include "CoreMinimal.h"
#include "Items/BaseItem.h"

namespace NilsHammondZombieHelpers
{
	inline bool IsWeapon(const ABaseItem* Item)
	{
		EItemType ItemType = Item->GetItemType();
		if (ItemType == EItemType::Shotgun || ItemType == EItemType::Pistol)
			return true;
		return false;
	}
}