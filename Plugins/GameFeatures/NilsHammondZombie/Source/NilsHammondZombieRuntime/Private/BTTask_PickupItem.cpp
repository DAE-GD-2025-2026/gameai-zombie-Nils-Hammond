#include "BTTask_PickupItem.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Items/BaseItem.h"
#include "Common/InventoryComponent.h"
#include "Survivor/SurvivorPawn.h"

UBTTask_PickupItem::UBTTask_PickupItem()
{
	NodeName = "Pickup item";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_PickupItem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Executing Pickup Task"));

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* Controller = OwnerComp.GetAIOwner();
	ASurvivorPawn* Pawn = Controller ? Cast<ASurvivorPawn>(Controller->GetPawn()) : nullptr;

	if (!BB || !Pawn) return EBTNodeResult::Failed;

	ABaseItem* Item = Cast<ABaseItem>(BB->GetValueAsObject(TargetItemKey));
	UInventoryComponent* Inventory = Pawn->GetComponentByClass<UInventoryComponent>();

	if (!Item || !Inventory) return EBTNodeResult::Failed;
	
	bool bPickedUp = false;
	for (int32 Slot = 0; Slot < Inventory->GetInventoryCapacity(); ++Slot)
	{
		if (Inventory->GrabItem(Slot, Item))
		{
			Inventory->UseItem(Slot);
			Inventory->RemoveItem(Slot);
			bPickedUp = true;
			break;
		}
	}

	if (bPickedUp)
	{
		
		BB->ClearValue(TargetItemKey);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}