#include "BTDecorator_LootAvailable.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTDecorator_LootAvailable::UBTDecorator_LootAvailable()
{
	NodeName = TEXT("Loot Available");
	FlowAbortMode = EBTFlowAbortMode::LowerPriority;

	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

bool UBTDecorator_LootAvailable::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;

	const bool bHasItem = BB->GetValueAsObject(NearestItemKey) != nullptr;
	const bool bHasHouse = BB->GetValueAsObject(NearestHouseKey) != nullptr;

	return bHasItem || bHasHouse;
}

void UBTDecorator_LootAvailable::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		BB->RegisterObserver(BB->GetKeyID(NearestItemKey), this,
			FOnBlackboardChangeNotification::CreateUObject(this, &UBTDecorator_LootAvailable::OnBlackboardKeyChanged));
		BB->RegisterObserver(BB->GetKeyID(NearestHouseKey), this,
			FOnBlackboardChangeNotification::CreateUObject(this, &UBTDecorator_LootAvailable::OnBlackboardKeyChanged));
	}
}

void UBTDecorator_LootAvailable::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		BB->UnregisterObserversFrom(this);
	}

	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

EBlackboardNotificationResult UBTDecorator_LootAvailable::OnBlackboardKeyChanged(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID) const
{
	return EBlackboardNotificationResult::ContinueObserving;
}