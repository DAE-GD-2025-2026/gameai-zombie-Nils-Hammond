#include "BTTask_Shoot.h"
#include "AIController.h"
#include "Common/InventoryComponent.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "NilsHammondZombieHelpers.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_Shoot::UBTTask_Shoot()
{
	NodeName = "Shoot";
}

bool UBTTask_Shoot::FireWeapon(AAIController& Controller, APawn& Pawn, UBlackboardComponent& BB)
{
	UInventoryComponent* Inventory = Pawn.GetComponentByClass<UInventoryComponent>();
	
	if (!Inventory)
		return false;
	
	TArray<ABaseItem*> CurrentItems = Inventory->GetInventory();
	ABaseItem* Item1 = !CurrentItems.IsEmpty() ? CurrentItems[0] : nullptr;
	ABaseItem* Item2 = CurrentItems.Num() > 1 ? CurrentItems[1] : nullptr;
		
	int WeakestWeaponSlot = 0;
	int Item1Ammo = Item1 && NilsHammondZombieHelpers::IsWeapon(Item1) ? Item1->GetValue() : INT_MAX;
	int Item2Ammo = Item2 && NilsHammondZombieHelpers::IsWeapon(Item2) ? Item2->GetValue() : INT_MAX;
		
	if (Item1Ammo > Item2Ammo)
		WeakestWeaponSlot = 1;
	
	if (!Inventory->UseItem(WeakestWeaponSlot))
		return false;
	
	if (CurrentItems[WeakestWeaponSlot]->GetValue() == 0)
	{
		Inventory->RemoveItem(WeakestWeaponSlot);
		int OtherWeaponSlot = (WeakestWeaponSlot + 1) % 2;
		if (!CurrentItems[OtherWeaponSlot] || !NilsHammondZombieHelpers::IsWeapon(CurrentItems[OtherWeaponSlot]))
		{
			BB.SetValueAsBool(TEXT("HasWeapon"), false);
		}
	}
	return true;
}

EBTNodeResult::Type UBTTask_Shoot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	AActor* Zombie = BB ? Cast<AActor>(BB->GetValueAsObject(ThreatKey)) : nullptr;

	if (!Pawn || !Zombie)
	{
		if (!Zombie)
			UE_LOG(LogTemp, Error, TEXT("Couldn't Find Zombie"));
		return EBTNodeResult::Failed;
	}

	const FVector PawnPos = Pawn->GetActorLocation();
	const FVector ZombiePos = Zombie->GetActorLocation();

	FCollisionQueryParams CollisionParams{};
	CollisionParams.AddIgnoredActor(Pawn);
	if (FHitResult HitResult{}; 
		GetWorld()->LineTraceSingleByChannel(HitResult, PawnPos, ZombiePos, 
			ECC_Pawn, CollisionParams) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find clear shot"));
		return EBTNodeResult::Failed;
	}

		
	FVector ToZombie = ZombiePos - PawnPos;
	ToZombie.Z = 0.f;
	const float Distance = ToZombie.Size();
	const FVector DirToThreat = ToZombie.GetSafeNormal();

	Pawn->SetActorRotation(ToZombie.Rotation());
	//Controller->SetFocus(Zombie);
	
	if (!FireWeapon(*Controller, *Pawn, *BB))
	{
		UE_LOG(LogTemp, Error, TEXT("Couldn't Fire Weapon"));
		return EBTNodeResult::Failed;
	}
	
	if (!IsValid(Zombie))
	{
		BB->ClearValue(ThreatKey);
	}
	return EBTNodeResult::Succeeded;
}