#include "BTTask_PickupItem.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Items/BaseItem.h"
#include "Common/InventoryComponent.h"
#include "Survivor/SurvivorPawn.h"
#include "NilsHammondZombieHelpers.h"

UBTTask_PickupItem::UBTTask_PickupItem()
{
	NodeName = "Pickup item";
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
	if (NilsHammondZombieHelpers::IsWeapon(Item))
	{
		int WeakestWeaponSlot = NilsHammondZombieHelpers::GetWeaponSlotWithLeastAmmo(CurrentItems, true);
		
		Inventory->UseItem(WeakestWeaponSlot);
		Inventory->RemoveItem(WeakestWeaponSlot);
		bPickedUp = Inventory->GrabItem(WeakestWeaponSlot, Item);
		
		if (bPickedUp)
			BB->SetValueAsBool(TEXT("HasWeapon"), true);
	}
	else
	{
		int ItemSlot = Inventory->GetInventoryCapacity() - 1;
		for (int i = 0; i < Inventory->GetInventoryCapacity(); i++)
		{
			if (CurrentItems[i] == nullptr)
			{
				ItemSlot = i;
				break;
			}
		}
		
		if (CurrentItems[ItemSlot] != nullptr)
		{
			Inventory->UseItem(ItemSlot);
			Inventory->RemoveItem(ItemSlot);
		}
		bPickedUp = Inventory->GrabItem(ItemSlot, Item);
		
		if (Item->GetItemType() == EItemType::Garbage)
			Inventory->RemoveItem(ItemSlot);
	}

	if (bPickedUp)
	{
		BB->ClearValue(TargetItemKey);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}