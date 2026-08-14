#include "BTTask_Wander.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_Wander::UBTTask_Wander()
{
	NodeName = "Wander";
	bNotifyTick = true;
}

bool UBTTask_Wander::PickNewWanderPoint(AAIController& Controller, APawn& Pawn, UBlackboardComponent* BB)
{
	const UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Pawn.GetWorld());
	if (!NavSys) return false;

	const FVector PawnPos = Pawn.GetActorLocation();
	FVector SampleOrigin = PawnPos;
	FVector AversionDir = FVector::ZeroVector;
	
	if (BB)
	{
		if (AActor* Zombie = Cast<AActor>(BB->GetValueAsObject(TEXT("NearestZombie"))))
		{
			FVector Away = PawnPos - Zombie->GetActorLocation();
			if (!Away.IsNearlyZero())
			{
				AversionDir += Away.GetSafeNormal() * ThreatAversionWeight;
			}
		}
		
		if (AActor* PurgeZone = Cast<AActor>(BB->GetValueAsObject(TEXT("NearestPurgeZone"))))
		{
			FVector Away = PawnPos - PurgeZone->GetActorLocation();
			if (!Away.IsNearlyZero())
			{
				AversionDir += Away.GetSafeNormal() * PurgeAversionWeight;
			}
		}
	}
	
	if (!AversionDir.IsNearlyZero())
	{
		SampleOrigin += AversionDir.GetClampedToMaxSize(1.f) * WanderRadius * 0.5f;
	}
	
	FNavLocation Result;
	if (NavSys->GetRandomReachablePointInRadius(SampleOrigin, WanderRadius, Result))
	{
		Controller.MoveToLocation(Result.Location);
		ElapsedSinceLastPoint = 0.f;
		return true;
	}
	return false;
}

EBTNodeResult::Type UBTTask_Wander::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	
	if (!Controller || !Controller->GetPawn() || !PickNewWanderPoint(*Controller, *Pawn, OwnerComp.GetBlackboardComponent()))
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
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
