#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetRunning.generated.h"

UCLASS()
class NILSHAMMONDZOMBIERUNTIME_API UBTTask_SetRunning : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SetRunning();

	UPROPERTY(EditAnywhere, Category = "Running")
	bool bShouldRun = true;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};