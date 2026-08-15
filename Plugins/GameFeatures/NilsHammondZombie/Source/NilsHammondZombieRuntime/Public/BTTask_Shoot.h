#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Shoot.generated.h"

class UBlackboardComponent;

UCLASS()
class NILSHAMMONDZOMBIERUNTIME_API UBTTask_Shoot : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Shoot();

	UPROPERTY(EditAnywhere, Category = "Shoot")
	FName ThreatKey = "NearestZombie";
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
private:
	bool FireWeapon(AAIController& Controller, APawn& Pawn, UBlackboardComponent& BB);
};