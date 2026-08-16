#pragma once

#include "CoreMinimal.h"
#include "Items/BaseItem.h"
#include "NavigationSystem.h"

namespace NilsHammondZombieHelpers
{
	bool IsWeapon(const ABaseItem* Item);
	bool FindNavPointAwayFromDirection(const UNavigationSystemV1* NavSys,
										FNavLocation& OutResult,
										const FVector& Origin,
										const FVector& BaseDirection,
										float SampleDistance,
										float StartingAngle,
										int AngleAttempts);
	int GetWeaponSlotWithLeastAmmo(const TArray<ABaseItem*>& CurrentInventory, bool bAllowNonWeapon);
}
