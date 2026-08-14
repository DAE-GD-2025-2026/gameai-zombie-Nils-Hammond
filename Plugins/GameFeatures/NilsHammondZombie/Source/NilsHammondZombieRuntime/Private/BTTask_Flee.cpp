#include "BTTask_Flee.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Flee::UBTTask_Flee()
{
	NodeName = "Flee";
	bNotifyTick = true;
}

bool UBTTask_Flee::PickFleePoint(AAIController& Controller, APawn& Pawn, UBlackboardComponent& BB)
{
	if (!BB.IsVectorValueSet(ThreatLocationKey)) return false;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Pawn.GetWorld());
	if (!NavSys) return false;

	const FVector PawnPos = Pawn.GetActorLocation();
	const FVector ThreatPos = BB.GetValueAsVector(ThreatLocationKey);

	FVector AwayDir = PawnPos - ThreatPos;
	AwayDir.Z = 0;
	AwayDir.Normalize();
	
	// If it can't find a point on the navmesh (because it's trying to run off the map) increase angle radius
	constexpr int MaxAttempts = 8;
	for (int Attempt = 1; Attempt < MaxAttempts; Attempt++)
	{
		const float RandomAngle = FMath::RandRange(-AngleVarianceDegrees * Attempt, AngleVarianceDegrees * Attempt);
		const FVector OffsetDir = AwayDir.RotateAngleAxis(RandomAngle, FVector::UpVector);
		const FVector SamplePoint = PawnPos + OffsetDir * FleeSampleDistance;
		
		FNavLocation Result;
		if (NavSys->ProjectPointToNavigation(SamplePoint, Result))
		{
			Controller.MoveToLocation(Result.Location);
			ElapsedSinceLastPoint = 0.f;
			return true;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("Couldn't find Nav Point"));

	return false;
}

EBTNodeResult::Type UBTTask_Flee::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!Controller || !Pawn || !BB || !PickFleePoint(*Controller, *Pawn, *BB))
	{
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_Flee::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!Controller || !Pawn || !BB)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (BB->IsVectorValueSet(ThreatLocationKey))
	{
		const float DistToThreat = FVector::Dist(Pawn->GetActorLocation(), BB->GetValueAsVector(ThreatLocationKey));
		UE_LOG(LogTemp, Warning, TEXT("Distance: %f"), DistToThreat);

		if (DistToThreat >= FleeDistance)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}
	}
	else
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}

	ElapsedSinceLastPoint += DeltaSeconds;

	const EPathFollowingStatus::Type Status = Controller->GetMoveStatus();

	if (Status == EPathFollowingStatus::Idle || ElapsedSinceLastPoint > MaxTimePerPoint)
	{
		PickFleePoint(*Controller, *Pawn, *BB);
	}
}