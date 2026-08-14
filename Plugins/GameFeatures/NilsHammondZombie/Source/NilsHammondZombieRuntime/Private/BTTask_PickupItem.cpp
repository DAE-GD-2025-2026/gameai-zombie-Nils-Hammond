#include "BTTask_PickupItem.h"

#include <Programs/UnrealBuildAccelerator/Core/Public/UbaBase.h>

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Items/BaseItem.h"
#include "Common/InventoryComponent.h"
#include "Items/Weapon.h"
#include "Survivor/SurvivorPawn.h"

UBTTask_PickupItem::UBTTask_PickupItem()
{
	NodeName = "Pickup item";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_PickupItem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* Controller = OwnerComp.GetAIOwner();
	ASurvivorPawn* Pawn = Controller ? Cast<ASurvivorPawn>(Controller->GetPawn()) : nullptr;

	if (!BB || !Pawn) return EBTNodeResult::Failed;

	ABaseItem* Item = Cast<ABaseItem>(BB->GetValueAsObject(TargetItemKey));
	UInventoryComponent* Inventory = Pawn->GetComponentByClass<UInventoryComponent>();

	if (!Item || !Inventory) return EBTNodeResult::Failed;
	
	bool bPickedUp = false;
	
	TArray<ABaseItem*> CurrentItems = Inventory->GetInventory();

	if (CurrentItems.Contains(Item))
	{
		BB->ClearValue(TargetItemKey);
		return EBTNodeResult::Succeeded;
	}
	if (IsWeapon(Item))
	{
		ABaseItem* Item1 = !CurrentItems.IsEmpty() ? CurrentItems[0] : nullptr;
		ABaseItem* Item2 = CurrentItems.Num() > 1 ? CurrentItems[1] : nullptr;
		
		int WeakestWeaponSlot = 0;
		int Item1Ammo = Item1 && IsWeapon(Item1) ? Item1->GetValue() : -1;
		int Item2Ammo = Item2 && IsWeapon(Item2) ? Item2->GetValue() : -1;
		
		if (Item1Ammo > Item2Ammo)
			WeakestWeaponSlot = 1;
		
		Inventory->UseItem(WeakestWeaponSlot);
		Inventory->RemoveItem(WeakestWeaponSlot);
		bPickedUp = Inventory->GrabItem(WeakestWeaponSlot, Item);
	}
	else
	{
		int32 FilledCount = 0;
		for (ABaseItem* CurrentItem : CurrentItems)
		{
			if (CurrentItem != nullptr)
			{
				++FilledCount;
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Current Items: %d, Capacity %d"), FilledCount, Inventory->GetInventoryCapacity());
		// Check if full up on slots, if so use up last slot, then pick up in next free slot regardless
		int ItemSlot = 0;
		if (FilledCount < Inventory->GetInventoryCapacity())
		{
			ItemSlot = FilledCount;
			bPickedUp = Inventory->GrabItem(ItemSlot, Item);
		}
		else
		{
			ItemSlot = FilledCount - 1;
			Inventory->UseItem(ItemSlot);
			Inventory->RemoveItem(ItemSlot);
			bPickedUp = Inventory->GrabItem(ItemSlot, Item);
		}
		
		UE_LOG(LogTemp, Warning, TEXT("Interacting with slot no %d"), ItemSlot);
		
		if (Item->GetItemType() == EItemType::Garbage)
			Inventory->RemoveItem(ItemSlot);
		
		//for (int32 Slot = 0; Slot < Inventory->GetInventoryCapacity(); ++Slot)
		//{
		//	if (Inventory->GrabItem(Slot, Item))
		//	{
		//		Inventory->UseItem(Slot);
		//		Inventory->RemoveItem(Slot);
		//		bPickedUp = true;
		//		break;
		//	}
		//}
	}

	const FString PickedUpStatus = bPickedUp ? TEXT("Picked Up == True") : TEXT("Picked Up == false");
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, PickedUpStatus);
	if (bPickedUp)
	{
		BB->ClearValue(TargetItemKey);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

bool UBTTask_PickupItem::IsWeapon(const ABaseItem* Item)
{
	EItemType ItemType = Item->GetItemType();
	if (ItemType == EItemType::Shotgun || ItemType == EItemType::Pistol)
		return true;
	return false;
}
