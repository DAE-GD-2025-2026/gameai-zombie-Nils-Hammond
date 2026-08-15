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

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	EBlackboardNotificationResult OnBlackboardKeyChanged(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID) const;
};