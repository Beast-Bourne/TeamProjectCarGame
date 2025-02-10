// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CarForcesComponent.generated.h"

USTRUCT(BlueprintType)
struct FWheelLoads
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float FrontRight;
	UPROPERTY(BlueprintReadOnly)
	float FrontLeft;
	UPROPERTY(BlueprintReadOnly)
	float RearRight;
	UPROPERTY(BlueprintReadOnly)
	float RearLeft;
	
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TEAMPROJECTCARGAME_API UCarForcesComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// constructor
	UCarForcesComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="CarForces")
	FWheelLoads CalculateStaticWheelLoads();

	UFUNCTION(BlueprintCallable, Category="CarForces")
	FWheelLoads CalculateSlopedWheelLoads(float bankAngle, float gradientAngle, float accel);

	UPROPERTY(BlueprintReadOnly)
	FWheelLoads wheelLoads; // loads on each tire in kg

	UPROPERTY(BlueprintReadOnly)
	float totalWheelLoadToCarWeightRatio;

	UPROPERTY(BlueprintReadWrite)
	float longitudinalAcceleration;

	UPROPERTY(BlueprintReadOnly)
	float lateralAcceleration;
	
private:
	// non constants
	float velocity = 0.0f; // vehicles forward velocity in m/s
	float acceleration = 0.0f; // vehicles forward acceleration in m/s^2
	float gradient = 0.0f; // vehicles pitch in radians (positive when the front is above the rear)
	float bank = 0.0f; // vehicles roll in radians (positive when the left is above the right)
	
	// Main body constants
	const float mass = 1450.0f; // mass in Kg
	const float centreOfMassHeight = 0.48f; // height of the centre of mass above the ground in meters
	const float wheelSeparation = 2.457f; // horizontal distance between the front and rear axles in meters
	const float frontWheelOffset = 0.957f; // horizontal offset from the centre of gravity in meters
	const float rearWheelOffset = 1.5f;
	const float frontTrackWidth = 1.6f; // the width of the front axle in meters
	const float rearTrackWidth = 1.56f;
	const float rollCentreHeight = 0.1f; // height of the roll centre above the ground in meters
	const float frontSuspensionStiffness = 120000.0f; // suspension in N/m
	const float rearSuspensionStiffness = 200000.0f;
	const float momentOfInertiaX = 2000.0f; // moment of inertia about the X axis in kg m^2
	const float momentOfInertiaY = 3000.0f;
	const float momentOfInertiaZ = 5000.0f;
	const float frontArea = 2.2f; // frontal area in m^2
	const float dragCoefficient = 0.39f; // the drag coefficient used for calculating the air resistance
	const float rollResistanceCoefficient = 0.012f; // the rolling resistance coefficient used for calculating the roll resistance force
	const float airDensity = 1.225f; // density of the air in kg/m^3
	const float g = 9.81f; // gravity

	// Tire constants
	const FWheelLoads staticWheelLoads = CalculateStaticWheelLoads();
	const float tireRadius = 0.350f; // tire radius in meters
	const float tireInertia = 2.2f; // the moment of inertia of the tires in kg m^2
	const float relaxationLengthCoefficient = 0.5f;
	const float threadStiffness = 30.0f; // N/m
	const float casterOffset = 0.03f; // m
	const float contactPatchLength = 0.1f; // m
	const float slidingFrictionConstant = 0.7f; // for dry asphalt

	// Tire load functions
	float CalculateLoadChangeFromLateralForce(float lateralForce, bool forFrontAxel);
	float CalculateWheelLoadRatio();

	// Longitudinal force functions
	float calculateResistanceForce();
};
