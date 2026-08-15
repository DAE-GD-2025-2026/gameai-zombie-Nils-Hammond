#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_UseConsumable.generated.h"

class ABaseItem;

UCLASS()
class NILSHAMMONDZOMBIERUNTIME_API UBTTask_UseConsumable : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_UseConsumable();

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName HealthRatioKey = "HealthRatio";
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName StaminaRatioKey = "StaminaRatio";
	
	UPROPERTY(EditAnywhere, Category = "UseConsumable")
	float UseThreshold = 0.75f;
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	TWeakObjectPtr<ABaseItem> TargetItem;
	bool bMoveIssued = false;
};