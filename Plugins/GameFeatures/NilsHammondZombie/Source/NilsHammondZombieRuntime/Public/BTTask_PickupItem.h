#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PickupItem.generated.h"

class ABaseItem;

UCLASS()
class NILSHAMMONDZOMBIERUNTIME_API UBTTask_PickupItem : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_PickupItem();

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName TargetItemKey = "NearestItem";
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName HasWeaponKey = "HasWeapon";

	UPROPERTY(EditAnywhere, Category = "Pickup")
	float AcceptanceRadius = 50.f;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	bool IsWeapon(const ABaseItem* Item);
	TWeakObjectPtr<ABaseItem> TargetItem;
	bool bMoveIssued = false;
};