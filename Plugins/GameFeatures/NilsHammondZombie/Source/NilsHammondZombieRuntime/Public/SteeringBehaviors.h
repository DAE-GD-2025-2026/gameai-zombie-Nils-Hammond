#pragma once

#include "CoreMinimal.h"
#include "SteeringHelpers.h"
#include "GameFramework/Pawn.h"

class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringOutput CalculateSteering(float DeltaT, APawn& Agent) = 0;

	virtual void SetTarget(const FTargetData& NewTarget) { Target = NewTarget; }
	
	template<class T, std::enable_if_t<std::is_base_of_v<ISteeringBehavior, T>>* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	FTargetData Target;
};

class FSeek : public ISteeringBehavior
{
public:
	FSeek() = default;
	virtual ~FSeek() override = default;

	virtual SteeringOutput CalculateSteering(float DeltaT, APawn& Agent) override;
};

class FFlee : public ISteeringBehavior
{
public:
	FFlee() = default;
	virtual ~FFlee() override = default;
	virtual SteeringOutput CalculateSteering(float DeltaT, APawn& Agent) override;
};

/*
class FArrive : public ISteeringBehavior
{
public:
	FArrive() = default;
	virtual ~FArrive() override = default;
	virtual SteeringOutput CalculateSteering(float DeltaT, APawn& Agent) override;
private:
	float OriginalMaxSpeed = -1.f;
};

class FFace : public ISteeringBehavior
{
public:
	FFace() = default;
	virtual ~FFace() override = default;
	virtual SteeringOutput CalculateSteering(float DeltaT, APawn& Agent) override;
};

class FPursuit : public ISteeringBehavior
{
public:
	FPursuit() = default;
	virtual ~FPursuit() override = default;
	virtual SteeringOutput CalculateSteering(float DeltaT, APawn& Agent) override;
};

class FEvade : public ISteeringBehavior
{
public:
	FEvade() = default;
	virtual ~FEvade() override = default;
	virtual SteeringOutput CalculateSteering(float DeltaT, APawn& Agent) override;
private:
	float EvadeRadius = 300.f;
};
*/

class FWander : public FSeek
{
public:
	FWander() = default;
	virtual ~FWander() override = default;

	virtual SteeringOutput CalculateSteering(float DeltaT, APawn& Agent) override;

	void SetWanderOffset(const float offset) { OffsetDistance = offset; }
	void SetWanderRadius(const float radius) { Radius = radius; }
	void SetMaxAngleChange(const float rad) { MaxAngleChange = rad; }
private:
	float OffsetDistance = 120.f;
	float Radius = 80.f;
	float MaxAngleChange = PI / 180.f * 45.f;
	float WanderAngle = 0.f;
};