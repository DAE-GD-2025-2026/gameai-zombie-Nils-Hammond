#include "BTTask_Wander.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

UBTTask_Wander::UBTTask_Wander()
{
	NodeName = "Wander";
	bNotifyTick = true;
}


EBTNodeResult::Type UBTTask_Wander::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Executing Task"));
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller || !Controller->GetPawn())
	{
		return EBTNodeResult::Failed;
	}

	if (!pWanderBehavior)
		pWanderBehavior = MakeUnique<FWander>();
	
	return EBTNodeResult::InProgress;
}

void UBTTask_Wander::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;

	if (!Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (!pWanderBehavior)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	const SteeringOutput Steering = pWanderBehavior->CalculateSteering(DeltaSeconds, *Pawn);

	if (!Steering.LinearVelocity.IsNearlyZero())
	{
		Pawn->AddMovementInput(FVector(Steering.LinearVelocity, 0.f), 1.f);
		FRotator newRotation = Pawn->GetControlRotation();
		newRotation.Yaw = Steering.AngularVelocity * DeltaSeconds;
		Pawn->SetActorRotation(newRotation);
	}
}
