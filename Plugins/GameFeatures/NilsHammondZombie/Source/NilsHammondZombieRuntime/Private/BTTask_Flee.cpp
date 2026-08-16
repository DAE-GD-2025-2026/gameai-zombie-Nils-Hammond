#include "BTTask_Flee.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "NilsHammondZombieHelpers.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Flee::UBTTask_Flee()
{
	NodeName = "Flee";
	bNotifyTick = true;
}

bool UBTTask_Flee::PickFleePoint(AAIController& Controller, APawn& Pawn, UBlackboardComponent& BB)
{
	AActor* FleeTarget = Cast<AActor>(BB.GetValueAsObject(FleeTargetKey.SelectedKeyName));

	if (!FleeTarget)
	{
		return false;
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Pawn.GetWorld());
	if (!NavSys)
	{
		return false;
	}

	const FVector PawnPos = Pawn.GetActorLocation();
	const FVector ThreatPos = FleeTarget->GetActorLocation();

	FVector AwayDir = PawnPos - ThreatPos;
	AwayDir.Z = 0;
	AwayDir.Normalize();
	
	FNavLocation Result;
	if (!NilsHammondZombieHelpers::FindNavPointAwayFromDirection(NavSys, Result, PawnPos,
		AwayDir, FleeSampleDistance, AngleVarianceDegrees, 8))
	{
		UE_LOG(LogTemp, Error, TEXT("Couldn't find Nav Point"));
		return false;
	}

	Controller.MoveToLocation(Result.Location);
	ElapsedSinceLastPoint = 0.f;
	return true;
}

EBTNodeResult::Type UBTTask_Flee::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!Controller || !Pawn || !BB || !PickFleePoint(*Controller, *Pawn, *BB))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed Flee"));
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_Flee::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	AActor* FleeTarget = BB ? Cast<AActor>(BB->GetValueAsObject(FleeTargetKey.SelectedKeyName)) : nullptr;
	if (!Controller || !Pawn || !BB || !FleeTarget)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	const float DistToThreat = FVector::Dist(Pawn->GetActorLocation(), FleeTarget->GetActorLocation());

	if (DistToThreat >= FleeDistance)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	ElapsedSinceLastPoint += DeltaSeconds;

	const EPathFollowingStatus::Type Status = Controller->GetMoveStatus();

	if (Status == EPathFollowingStatus::Idle || ElapsedSinceLastPoint > MaxTimePerPoint)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}