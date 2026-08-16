#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Flee.generated.h"

class UBlackboardComponent;

UCLASS()
class NILSHAMMONDZOMBIERUNTIME_API UBTTask_Flee : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Flee();

	UPROPERTY(EditAnywhere, Category = "Flee")
	FBlackboardKeySelector FleeTargetKey;

	UPROPERTY(EditAnywhere, Category = "Flee")
	float FleeDistance = 800.f;

	UPROPERTY(EditAnywhere, Category = "Flee")
	float FleeSampleDistance = 1200.f;

	UPROPERTY(EditAnywhere, Category = "Flee")
	float AngleVarianceDegrees = 20.f;

	UPROPERTY(EditAnywhere, Category = "Flee")
	float MaxTimePerPoint = 10.f;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	bool PickFleePoint(AAIController& Controller, APawn& Pawn, UBlackboardComponent& BB);

	float ElapsedSinceLastPoint = 0.f;
};