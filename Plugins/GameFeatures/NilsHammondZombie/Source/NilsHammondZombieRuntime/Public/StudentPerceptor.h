// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense_Damage.h"
#include "StudentPerceptor.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NILSHAMMONDZOMBIERUNTIME_API UStudentPerceptor : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UStudentPerceptor();
	
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION()
	virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	
private:
	AActor* FindNearest(TArray<TObjectPtr<AActor>> Actors) const;
	bool IsHouseOnCooldown(AActor* House);
	void MarkHouseVisited(AActor* House);
	void CheckHouseOccupancy();
	
	UPROPERTY()
	TObjectPtr<UAIPerceptionComponent> PerceptionComp;
	
	UPROPERTY()
	TObjectPtr<UBlackboardComponent> BlackboardComp;
	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> KnownItems;
	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> KnownHouses;
	
	UPROPERTY()
	TMap<TObjectPtr<AActor>, float> RecentlyVisitedHouses;
	
	UPROPERTY()
	float HouseRevisitCooldown = 60.f;
	
	UPROPERTY()
	TObjectPtr<AActor> CurrentHouse;
	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> KnownPurgeZones;
	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> KnownZombies;
	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> KnownDamagingZombies;
};
