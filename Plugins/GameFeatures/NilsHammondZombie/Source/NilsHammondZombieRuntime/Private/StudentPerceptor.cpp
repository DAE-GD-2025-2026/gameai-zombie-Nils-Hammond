// Fill out your copyright notice in the Description page of Project Settings.


#include "StudentPerceptor.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Items/BaseItem.h"
#include "PurgeZones/PurgeZone.h"
#include "Village/House/House.h"
#include "Zombies/BaseZombie.h"

namespace BBKeys
{
	const FName NearestItem = TEXT("NearestItem");
	const FName NearestItemType = TEXT("NearestItemType");
	const FName NearestHouse = TEXT("NearestHouse");
	const FName CurrentHouse = TEXT("CurrentHouse");
	const FName NearestPurgeZone = TEXT("NearestPurgeZone");
	const FName NearestZombie = TEXT("NearestZombie");
	const FName LastKnownThreatLocation = TEXT("LastKnownThreatLocation");
	const FName HealthRatio = TEXT("HealthRatio");
	const FName StaminaRatio = TEXT("StaminaRatio");
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
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &UStudentPerceptor::OnPerceptionUpdated);
	}
	
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (AAIController* Controller = Cast<AAIController>(Pawn->GetController()))
		{
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
	TArray<TObjectPtr<AActor>> ValidHouses = KnownHouses.FilterByPredicate([this](AActor* House)
	{
		return !IsHouseOnCooldown(House) && House != CurrentHouse;
	});
	BlackboardComp->SetValueAsObject(BBKeys::NearestHouse, FindNearest(ValidHouses));
	CheckHouseOccupancy();

	BlackboardComp->SetValueAsObject(BBKeys::NearestItem, FindNearest(KnownItems));
	BlackboardComp->SetValueAsObject(BBKeys::NearestPurgeZone, FindNearest(KnownPurgeZones));
	
	TSet<TObjectPtr<AActor>> AllKnownZombies(KnownZombies);
	AllKnownZombies.Append(KnownDamagingZombies);
	AActor* NearestZombie = FindNearest(AllKnownZombies.Array());
	BlackboardComp->SetValueAsObject(BBKeys::NearestZombie, NearestZombie);
	if (NearestZombie)
		BlackboardComp->SetValueAsVector(BBKeys::LastKnownThreatLocation, NearestZombie->GetActorLocation());
	
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
		return;
	if (UHealthComponent* HPComponent = Pawn->GetComponentByClass<UHealthComponent>())
	{
		BlackboardComp->SetValueAsFloat(BBKeys::HealthRatio, HPComponent->GetHealth() / HPComponent->GetMaxHealth());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Couldn't Find Health Component"));
	}
	if (UStaminaComponent* StaminaComponent = Pawn->GetComponentByClass<UStaminaComponent>())
	{
		BlackboardComp->SetValueAsFloat(BBKeys::StaminaRatio, StaminaComponent->GetCurrentStamina() / StaminaComponent->GetMaxStamina());
	}
}

void UStudentPerceptor::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		if (Cast<ABaseZombie>(Actor))
		{
			KnownDamagingZombies.Add(Actor);
			KnownDamagingZombies = KnownDamagingZombies.FilterByPredicate([this](AActor* Zombie)
			{
				return FVector::DistSquared(GetOwner()->GetActorLocation(), Zombie->GetActorLocation()) < 10000;
			});
		}
	}
	
	KnownItems.Empty();
	KnownHouses.Empty();
	KnownZombies.Empty();
	KnownPurgeZones.Empty();
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
		if (Cast<ABaseZombie>(PerceivedActor))
		{
			KnownZombies.Add(PerceivedActor);
		}
		if (Cast<APurgeZone>(PerceivedActor))
		{
			KnownPurgeZones.Add(PerceivedActor);
		}
	}
}

AActor* UStudentPerceptor::FindNearest(TArray<TObjectPtr<AActor>> Actors) const
{
	if (!BlackboardComp)
		return nullptr;
	
	Actors.RemoveAll([](AActor* A) { return !IsValid(A) || A->IsHidden(); });
	
	AActor* Nearest = nullptr;
	float BestDistSq = FLT_MAX;
	const FVector OwnerPos = GetOwner()->GetActorLocation();

	for (AActor* Actor : Actors)
	{
		const float DistSq = FVector::DistSquared(OwnerPos, Actor->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Nearest = Actor;
		}
	}

	return Nearest;
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