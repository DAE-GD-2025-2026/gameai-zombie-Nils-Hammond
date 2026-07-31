#include "SteeringBehaviors.h"

//SEEK
SteeringOutput FSeek::CalculateSteering(float DeltaT, APawn& Agent)
{
	SteeringOutput Steering{};
	Steering.LinearVelocity = Target.Position - FVector2D(Agent.GetActorLocation());
	Steering.LinearVelocity.Normalize();

	return Steering;
}

//FLEE
SteeringOutput FFlee::CalculateSteering(float DeltaT, APawn& Agent)
{
	SteeringOutput Steering{};
	Steering.LinearVelocity = FVector2D(Agent.GetActorLocation()) - Target.Position;
	Steering.LinearVelocity.Normalize();

	return Steering;
}

//WANDER
SteeringOutput FWander::CalculateSteering(float DeltaT, APawn& Agent)
{
	WanderAngle += FMath::RandRange(-MaxAngleChange / 2.f, MaxAngleChange / 2.f);
	const FVector2D CircleCenter = FVector2D(Agent.GetActorLocation()) + FVector2D(Agent.GetActorForwardVector().X, Agent.GetActorForwardVector().Y) * OffsetDistance;
	const FVector2D Displacement = FVector2D(FMath::Cos(WanderAngle), FMath::Sin(WanderAngle)) * Radius;

	Target.Position = CircleCenter + Displacement;

	SteeringOutput Steering{};
	Steering = FSeek::CalculateSteering(DeltaT, Agent);
	
	{
		DrawDebugCircle(Agent.GetWorld(), 
			FVector(CircleCenter, 0), 
			Radius, 
			32, 
			FColor::Cyan, 
			false, 
			-1.f, 
			0, 
			1.f, 
			FVector(0, 1, 0), 
			FVector(1, 0, 0));
        
		DrawDebugLine(Agent.GetWorld(), 
			Agent.GetActorLocation(), 
			FVector(CircleCenter, 0), 
			FColor::Cyan, 
			false, 
			-1.f, 
			0, 
			1.f);
        
		DrawDebugSphere(Agent.GetWorld(), 
			FVector(Target.Position, 0), 
			15.f, 
			8, 
			FColor::Magenta);
        
		DrawDebugLine(Agent.GetWorld(), 
			Agent.GetActorLocation(), 
			FVector(Target.Position, 0), 
			FColor::Magenta, 
			false, 
			-1.f, 
			0, 
			2.f);
	}

	return Steering;
}
