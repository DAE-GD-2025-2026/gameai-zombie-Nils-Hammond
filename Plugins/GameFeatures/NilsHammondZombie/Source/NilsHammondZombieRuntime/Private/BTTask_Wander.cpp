#include "BTTask_Wander.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "NilsHammondZombieHelpers.h"
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
	FVector AversionDir = FVector::ZeroVector;
	float CombinedCloseness = 0.f;
	
	if (BB)
	{
		if (AActor* Zombie = Cast<AActor>(BB->GetValueAsObject(TEXT("NearestZombie"))))
		{
			FVector Away = PawnPos - Zombie->GetActorLocation();
			Away.Z = 0.f;
			const float Dist = Away.Size();
			if (Dist < WanderRadius && !Away.IsNearlyZero())
			{
				AversionDir += Away.GetSafeNormal() * ThreatAversionWeight;
				const float Closeness = (1.f - (Dist / WanderRadius)) * ThreatAversionWeight;
				CombinedCloseness += Closeness;
			}
		}
		
		if (AActor* PurgeZone = Cast<AActor>(BB->GetValueAsObject(TEXT("NearestPurgeZone"))))
		{
			FVector Away = PawnPos - PurgeZone->GetActorLocation();
			Away.Z = 0;
			const float Dist = Away.Size();
			if (Dist < WanderRadius && !Away.IsNearlyZero())
			{
				AversionDir += Away.GetSafeNormal() * PurgeAversionWeight;
				const float Closeness = (1.f - (Dist / WanderRadius)) * PurgeAversionWeight;
				CombinedCloseness += Closeness;
			}
		}
	}
	
	
	constexpr float MinAngle = 15.f;
	const float MaxAngle = FMath::Lerp(180.f, MinAngle, CombinedCloseness);
	CombinedCloseness = FMath::Clamp(CombinedCloseness, 0.f, 1.f);
	AversionDir.Normalize();
	
	FNavLocation Result;
	if (!NilsHammondZombieHelpers::FindNavPointAwayFromDirection(NavSys, Result, PawnPos,
		AversionDir, WanderRadius, MaxAngle, 4))
	{
		return false;
	}
	
	Controller.MoveToLocation(Result.Location);
	ElapsedSinceLastPoint = 0.f;
	return true;
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
