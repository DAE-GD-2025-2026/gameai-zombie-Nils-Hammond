#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Wander.generated.h"

UCLASS()
class NILSHAMMONDZOMBIERUNTIME_API UBTTask_Wander : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Wander();

	UPROPERTY(EditAnywhere, Category = "Wander")
	float WanderRadius = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Wander")
	float MaxTimePerPoint = 15.f;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	bool PickNewWanderPoint(AAIController& Controller, APawn& Pawn);

	float ElapsedSinceLastPoint = 0.f;
};