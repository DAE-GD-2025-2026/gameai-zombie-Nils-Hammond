#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_LootAvailable.generated.h"

UCLASS()
class NILSHAMMONDZOMBIERUNTIME_API UBTDecorator_LootAvailable : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_LootAvailable();

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName NearestItemKey = "NearestItem";

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName NearestHouseKey = "NearestHouse";


protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

private:
	mutable bool bLastConditionValue = false;
};