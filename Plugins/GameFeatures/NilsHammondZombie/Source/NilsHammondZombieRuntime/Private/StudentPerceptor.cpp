// Fill out your copyright notice in the Description page of Project Settings.


#include "StudentPerceptor.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Items/BaseItem.h"


UStudentPerceptor::UStudentPerceptor()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStudentPerceptor::BeginPlay()
{
	Super::BeginPlay();
	
	if ((PerceptionComp = GetOwner()->GetComponentByClass<UAIPerceptionComponent>()))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Found Perception Comp"));

		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &UStudentPerceptor::OnPerceptionUpdated);
	}
	
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (AAIController* Controller = Cast<AAIController>(Pawn->GetController()))
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Found Blackboard Comp"));
			BlackboardComp = Controller->GetBlackboardComponent();
		}
	}
}

void UStudentPerceptor::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	PerceivedItems.RemoveAll([](AActor* A) { return !IsValid(A); });
	ReevaluateNearestItem();
	
	if (GEngine)
	{
		const int32 BaseKey = 1000; // arbitrary offset so it doesn't collide with other debug messages

		GEngine->AddOnScreenDebugMessage(BaseKey, 1.f, FColor::Cyan,
			FString::Printf(TEXT("Perceived Items: %d"), PerceivedItems.Num()));

		for (int32 i = 0; i < PerceivedItems.Num(); ++i)
		{
			const AActor* Item = PerceivedItems[i];
			GEngine->AddOnScreenDebugMessage(BaseKey + 1 + i, 1.f, FColor::White,
				FString::Printf(TEXT("  [%d] %s"), i, Item ? *Item->GetName() : TEXT("null")));
		}
	}
}

void UStudentPerceptor::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;

	//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
	//	FString::Printf(TEXT("Saw: %s (Class: %s)"), *Actor->GetName(), *Actor->GetClass()->GetName()));
	
	if (Cast<ABaseItem>(Actor))
	{
		if (Stimulus.WasSuccessfullySensed())
			PerceivedItems.Add(Actor);
		else
			PerceivedItems.Remove(Actor);
	}
}

void UStudentPerceptor::ReevaluateNearestItem()
{
	if (!BlackboardComp)
		return;
	
	AActor* Nearest = nullptr;
	float BestDistSq = FLT_MAX;
	const FVector OwnerPos = GetOwner()->GetActorLocation();

	for (AActor* Item : PerceivedItems)
	{
		const float DistSq = FVector::DistSquared(OwnerPos, Item->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Nearest = Item;
		}
	}
	
	BlackboardComp->SetValueAsObject(TEXT("NearestItem"), Nearest);
}