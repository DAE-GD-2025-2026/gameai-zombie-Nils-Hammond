#include "BTTask_Wander.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_Wander::UBTTask_Wander()
{
	NodeName = "Wander";
	bNotifyTick = true;
}

bool UBTTask_Wander::PickNewWanderPoint(AAIController& Controller, APawn& Pawn)
{
	const UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Pawn.GetWorld());
	if (!NavSys) return false;

	FNavLocation Result;
	if (NavSys->GetRandomReachablePointInRadius(Pawn.GetActorLocation(), WanderRadius, Result))
	{
		Controller.MoveToLocation(Result.Location);
		ElapsedSinceLastPoint = 0.f;
		return true;
	}
	return false;
}

EBTNodeResult::Type UBTTask_Wander::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//if (GEngine)
	//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Executing Wander Task"));

	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	
	if (!Controller || !Controller->GetPawn() || !PickNewWanderPoint(*Controller, *Pawn))
	{
		return EBTNodeResult::Failed;
	}
	
	return EBTNodeResult::InProgress;
}

void UBTTask_Wander::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;

	if (!Controller || !Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	ElapsedSinceLastPoint += DeltaSeconds;
	
	const EPathFollowingStatus::Type Status = Controller->GetMoveStatus();
	if (Status == EPathFollowingStatus::Idle || ElapsedSinceLastPoint > MaxTimePerPoint)
	{
		PickNewWanderPoint(*Controller, *Pawn);
	}
}
