#include "BTTask_UseConsumable.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Common/InventoryComponent.h"
#include "Items/BaseItem.h"
#include "Survivor/SurvivorPawn.h"

UBTTask_UseConsumable::UBTTask_UseConsumable()
{
	NodeName = "Use Consumable";
}

EBTNodeResult::Type UBTTask_UseConsumable::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* Controller = OwnerComp.GetAIOwner();
	ASurvivorPawn* Pawn = Controller ? Cast<ASurvivorPawn>(Controller->GetPawn()) : nullptr;

	if (!BB || !Pawn) return EBTNodeResult::Failed;

	UInventoryComponent* Inventory = Pawn->GetComponentByClass<UInventoryComponent>();

	if (!Inventory) return EBTNodeResult::Failed;

	EItemType DesiredItemType = EItemType::Food;
	if (BB->GetValueAsFloat(HealthRatioKey) <= UseThreshold)
	{
		DesiredItemType = EItemType::Medkit;
	}
	else if (BB->GetValueAsFloat(StaminaRatioKey) > UseThreshold)
	{
		UE_LOG(LogTemp, Warning, TEXT("Vitals are fine"));

		// Both Health and Stamina are fine enough
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogTemp, Warning, TEXT("Ratio at %f, Need %s"),
		DesiredItemType == EItemType::Food ? BB->GetValueAsFloat(StaminaRatioKey) : BB->GetValueAsFloat(HealthRatioKey),
		DesiredItemType == EItemType::Food ? TEXT("Food") : TEXT("Medkit"));

	TArray<ABaseItem*> CurrentItems = Inventory->GetInventory();
	for (int SlotIdx = 0; SlotIdx < CurrentItems.Num(); ++SlotIdx)
	{
		if (CurrentItems[SlotIdx] && CurrentItems[SlotIdx]->GetItemType() == DesiredItemType)
		{
			UE_LOG(LogTemp, Warning, TEXT("Consuming Slot %d"), SlotIdx);
			Inventory->UseItem(SlotIdx);
			Inventory->RemoveItem(SlotIdx);
			return EBTNodeResult::Succeeded;
		}
	}
	
	return EBTNodeResult::Failed;
}