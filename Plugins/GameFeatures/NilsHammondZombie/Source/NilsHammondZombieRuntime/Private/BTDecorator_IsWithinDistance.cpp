#include "BTDecorator_IsWithinDistance.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_IsWithinDistance::UBTDecorator_IsWithinDistance()
{
	NodeName = TEXT("Is Within Distance");
	FlowAbortMode = EBTFlowAbortMode::LowerPriority;
	bNotifyTick = true;
}

bool UBTDecorator_IsWithinDistance::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!Controller || !Pawn || !BB)
		return false;

	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor)
		return false;
	
	const float Distance = FVector::Dist(Pawn->GetActorLocation(), TargetActor->GetActorLocation());

	return Distance < TargetDistance;
}

void UBTDecorator_IsWithinDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const bool bCurrent = CalculateRawConditionValue(OwnerComp, NodeMemory);
	if (bCurrent != bLastConditionValue)
	{
		bLastConditionValue = bCurrent;
		OwnerComp.RequestExecution(this);
	}
}