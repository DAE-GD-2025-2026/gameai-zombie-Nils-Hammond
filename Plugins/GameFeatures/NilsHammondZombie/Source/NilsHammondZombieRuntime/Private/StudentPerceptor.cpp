// Fill out your copyright notice in the Description page of Project Settings.


#include "StudentPerceptor.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Items/BaseItem.h"
#include "Village/House/House.h"

namespace BBKeys
{
	const FName NearestItem = TEXT("NearestItem");
	const FName NearestItemType = TEXT("NearestItemType");
	const FName NearestHouse = TEXT("NearestHouse");
	const FName CurrentHouse = TEXT("CurrentHouse");
	const FName NearestPurgeZone = TEXT("NearestPurgeZone");
	const FName NearestZombie = TEXT("NearestZombie");
}

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
	
	const float Now = GetWorld()->GetTimeSeconds();
	for (auto It = RecentlyVisitedHouses.CreateIterator(); It; ++It)
	{
		if (!IsValid(It->Key) || Now - It->Value > HouseRevisitCooldown)
		{
			It.RemoveCurrent();
		}
	}

	ReevaluateNearestHouse();
	KnownItems.RemoveAll([](AActor* A) { return !IsValid(A) || A->IsHidden(); });
	ReevaluateNearestItem();
	
	CheckHouseOccupancy();
	
	if (GEngine)
	{
		const int32 BaseKey = 1000;

		GEngine->AddOnScreenDebugMessage(BaseKey, 1.f, FColor::Cyan,
			FString::Printf(TEXT("Perceived Items: %d"), KnownItems.Num()));

		for (int32 i = 0; i < KnownItems.Num(); ++i)
		{
			const AActor* Item = KnownItems[i];
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
	
	//if (Cast<ABaseItem>(Actor))
	//{
	//	if (Stimulus.WasSuccessfullySensed())
	//		PerceivedItems.Add(Actor);
	//	else
	//		PerceivedItems.Remove(Actor);
	//}
	KnownItems.Empty();
	KnownHouses.Empty();
	TArray<AActor*> PerceivedActors;
	PerceptionComp->GetKnownPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);
	for (AActor* PerceivedActor : PerceivedActors)
	{
		if (PerceivedActor->IsHidden())
			continue;
		if (Cast<ABaseItem>(PerceivedActor))
		{
			KnownItems.Add(PerceivedActor);
		}
		if (Cast<AHouse>(PerceivedActor))
		{
			KnownHouses.Add(PerceivedActor);
		}
	}
}

void UStudentPerceptor::ReevaluateNearestItem()
{
	if (!BlackboardComp)
		return;
	
	AActor* Nearest = nullptr;
	float BestDistSq = FLT_MAX;
	const FVector OwnerPos = GetOwner()->GetActorLocation();

	for (AActor* Item : KnownItems)
	{
		const float DistSq = FVector::DistSquared(OwnerPos, Item->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Nearest = Item;
		}
	}
	
	if (!Nearest)
	{
		BlackboardComp->ClearValue(BBKeys::NearestItem);
		return;
	}

	ABaseItem* NearestItem = static_cast<ABaseItem*>(Nearest);
	
	BlackboardComp->SetValueAsObject(BBKeys::NearestItem, NearestItem);
	BlackboardComp->SetValueAsEnum(BBKeys::NearestItemType, static_cast<uint8>(NearestItem->GetItemType()));
}

void UStudentPerceptor::ReevaluateNearestHouse()
{
	if (!BlackboardComp)
		return;
	
	AActor* Nearest = nullptr;
	float BestDistSq = FLT_MAX;
	const FVector OwnerPos = GetOwner()->GetActorLocation();

	for (AActor* House : KnownHouses)
	{
		if (IsHouseOnCooldown(House) || House == CurrentHouse) continue;

		const float DistSq = FVector::DistSquared(OwnerPos, House->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Nearest = House;
		}
	}

	BlackboardComp->SetValueAsObject(BBKeys::NearestHouse, Nearest);
}

void UStudentPerceptor::MarkHouseVisited(AActor* House)
{
	if (House)
		RecentlyVisitedHouses.Add(House, GetWorld()->GetTimeSeconds());
}

bool UStudentPerceptor::IsHouseOnCooldown(AActor* House)
{
	if (!House)
		return false;
	
	if (const float* VisitedTime = RecentlyVisitedHouses.Find(House))
	{
		const float Elapsed = GetWorld()->GetTimeSeconds() - *VisitedTime;
		return Elapsed < HouseRevisitCooldown;
	}
	return false;
}

void UStudentPerceptor::CheckHouseOccupancy()
{
	if (!BlackboardComp || !GetOwner()) return;

	const FVector OwnerPos = GetOwner()->GetActorLocation();

	if (IsValid(CurrentHouse))
	{
		if (AHouse* House = Cast<AHouse>(CurrentHouse.Get()))
		{
			const FHouseBounds Bounds = House->GetBounds();
			const FBox HouseBox(Bounds.Origin - Bounds.Extent, Bounds.Origin + Bounds.Extent);

			if (!HouseBox.IsInside(OwnerPos))
			{
				MarkHouseVisited(CurrentHouse.Get());
				BlackboardComp->ClearValue(BBKeys::CurrentHouse);
				CurrentHouse = nullptr;
			}
		}
	}
	else
	{
		AActor* TargetHouse = Cast<AActor>(BlackboardComp->GetValueAsObject(BBKeys::NearestHouse));
		if (AHouse* House = Cast<AHouse>(TargetHouse))
		{
			const FHouseBounds Bounds = House->GetBounds();
			const FBox HouseBox(Bounds.Origin - Bounds.Extent, Bounds.Origin + Bounds.Extent);

			if (HouseBox.IsInside(OwnerPos))
			{
				CurrentHouse = TargetHouse;
				BlackboardComp->SetValueAsObject(BBKeys::CurrentHouse, TargetHouse);
			}
		}
	}
}