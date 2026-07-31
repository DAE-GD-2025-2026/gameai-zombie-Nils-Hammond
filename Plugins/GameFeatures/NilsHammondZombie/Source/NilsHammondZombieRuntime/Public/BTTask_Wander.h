#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SteeringBehaviors.h"
#include "BTTask_Wander.generated.h"

UCLASS()
class NILSHAMMONDZOMBIERUNTIME_API UBTTask_Wander : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Wander();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	TUniquePtr<FWander> pWanderBehavior;
};