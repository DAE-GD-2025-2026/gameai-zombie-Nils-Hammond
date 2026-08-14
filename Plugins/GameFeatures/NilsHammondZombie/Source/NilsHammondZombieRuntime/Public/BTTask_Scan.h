#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Scan.generated.h"

UCLASS()
class NILSHAMMONDZOMBIERUNTIME_API UBTTask_Scan : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Scan();

	UPROPERTY(EditAnywhere, Category = "Scan")
	float RotationSpeedDegreesPerSecond = 180.f;

	UPROPERTY(EditAnywhere, Category = "Scan")
	float TotalScanDegrees = 360.f;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	float DegreesRotatedSoFar = 0.f;
};