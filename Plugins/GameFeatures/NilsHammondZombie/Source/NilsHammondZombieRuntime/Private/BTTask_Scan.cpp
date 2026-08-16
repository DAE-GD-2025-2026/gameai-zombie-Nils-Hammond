#include "BTTask_Scan.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

UBTTask_Scan::UBTTask_Scan()
{
	NodeName = "Scan";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_Scan::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller || !Controller->GetPawn())
	{
		return EBTNodeResult::Failed;
	}

	DegreesRotatedSoFar = 0.f;
	return EBTNodeResult::InProgress;
}

void UBTTask_Scan::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;

	if (!Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	const float DeltaYaw = RotationSpeedDegreesPerSecond * DeltaSeconds;
	FRotator NewRotation = Pawn->GetActorRotation();
	NewRotation.Yaw += DeltaYaw;
	Pawn->SetActorRotation(NewRotation);

	DegreesRotatedSoFar += DeltaYaw;

	if (DegreesRotatedSoFar >= TotalScanDegrees)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}