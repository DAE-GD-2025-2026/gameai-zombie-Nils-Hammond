#include "BTDecorator_LootAvailable.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTDecorator_LootAvailable::UBTDecorator_LootAvailable()
{
	NodeName = TEXT("Loot Available");
	FlowAbortMode = EBTFlowAbortMode::LowerPriority;
	bNotifyTick = true;
}

bool UBTDecorator_LootAvailable::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;

	const bool bHasItem = BB->GetValueAsObject(NearestItemKey) != nullptr;
	const bool bHasHouse = BB->GetValueAsObject(NearestHouseKey) != nullptr;

	return bHasItem || bHasHouse;
}

void UBTDecorator_LootAvailable::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const bool bCurrent = CalculateRawConditionValue(OwnerComp, NodeMemory);
	if (bCurrent != bLastConditionValue)
	{
		bLastConditionValue = bCurrent;
		OwnerComp.RequestExecution(this);
	}
}