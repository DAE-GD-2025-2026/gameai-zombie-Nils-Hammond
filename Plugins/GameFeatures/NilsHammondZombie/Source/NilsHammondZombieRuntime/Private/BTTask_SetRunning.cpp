#include "BTTask_SetRunning.h"
#include "AIController.h"
#include "Survivor/SurvivorPawn.h"

UBTTask_SetRunning::UBTTask_SetRunning()
{
	NodeName = "Set Running";
}

EBTNodeResult::Type UBTTask_SetRunning::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	ASurvivorPawn* Pawn = Controller ? Cast<ASurvivorPawn>(Controller->GetPawn()) : nullptr;

	if (!Pawn)
		return EBTNodeResult::Failed;

	if (bShouldRun)
	{
		Pawn->StartRunning();
	}
	else
	{
		Pawn->StopRunning();
	}
	return EBTNodeResult::Succeeded;
}