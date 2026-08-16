#include "NilsHammondZombieHelpers.h"
#include "Items/BaseItem.h"

bool NilsHammondZombieHelpers::IsWeapon(const ABaseItem* Item)
{
	EItemType ItemType = Item->GetItemType();
	if (ItemType == EItemType::Shotgun || ItemType == EItemType::Pistol)
		return true;
	return false;
}

bool NilsHammondZombieHelpers::FindNavPointAwayFromDirection(const UNavigationSystemV1* NavSys, FNavLocation& OutResult, const FVector& Origin, const FVector& BaseDirection, float SampleDistance, float StartingAngle, int AngleAttempts)
{
	if (!NavSys)
		return false;

	FVector Dir = BaseDirection;
	Dir.Z = 0.f;
	if (Dir.IsNearlyZero())
		Dir = FVector::ForwardVector;
	Dir.Normalize();

	for (int Attempt = 1; Attempt < AngleAttempts + 1; Attempt++)
	{
		float MaxAngle = FMath::Min(StartingAngle * Attempt, 180);
		const float RandomAngle = FMath::RandRange(-MaxAngle, MaxAngle);
		const FVector OffsetDir = Dir.RotateAngleAxis(RandomAngle, FVector::UpVector);
		const FVector SamplePoint = Origin + OffsetDir * SampleDistance;
		
		UE_LOG(LogTemp, Error, TEXT("FindNavPoint: Moving in direction {%f, %f}"), OffsetDir.X, OffsetDir.Y);
		
		if (NavSys->ProjectPointToNavigation(SamplePoint, OutResult, FVector(500.f, 500.f, 500.f)))
		{
			return true;
		}
	}
	return false;
}

int NilsHammondZombieHelpers::GetWeaponSlotWithLeastAmmo(const TArray<ABaseItem*>& CurrentInventory, bool bAllowNonWeapon)
{
	const ABaseItem* Item1 = !CurrentInventory.IsEmpty() ? CurrentInventory[0] : nullptr;
	const ABaseItem* Item2 = CurrentInventory.Num() > 1 ? CurrentInventory[1] : nullptr;
	
	int WeakestWeaponSlot = 0;
	const int NonWeaponAmmoValue = bAllowNonWeapon ? -1 : INT_MAX;
	const int Item1Ammo = Item1 && NilsHammondZombieHelpers::IsWeapon(Item1) ? Item1->GetValue() : NonWeaponAmmoValue;
	const int Item2Ammo = Item2 && NilsHammondZombieHelpers::IsWeapon(Item2) ? Item2->GetValue() : NonWeaponAmmoValue;
		
	if (Item1Ammo > Item2Ammo)
		WeakestWeaponSlot = 1;
	
	return WeakestWeaponSlot;
}
