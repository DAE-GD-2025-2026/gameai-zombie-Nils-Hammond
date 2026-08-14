#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsWithinDistance.generated.h"

UCLASS()
class NILSHAMMONDZOMBIERUNTIME_API UBTDecorator_IsWithinDistance : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_IsWithinDistance();

	UPROPERTY(EditAnywhere, Category="Condition")
	FBlackboardKeySelector TargetActorKey;
	
	UPROPERTY(EditAnywhere, Category="Condition")
	float TargetDistance;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
private:
	mutable bool bLastConditionValue = false;
};