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
	// Main body constants
	const float mass = 1450.0f; // mass in Kg
	const float centreOfMassHeight = 0.48f; // height of the centre of mass above the ground in meters
	const float wheelSeparation = 2.457f; // horizontal distance between the front and rear axles in meters
	const float frontWheelOffset = 0.957f; // horizontal offset from the centre of gravity in meters
	const float rearWheelOffset = 1.5f;
	const float frontTrackWidth = 1.6f; // the width of the front axle in meters
	const float rearTrackWidth = 1.56f;
	const float rollCentreHeight = 0.1f;
	const float frontSuspensionStiffness = 120000.0f; // suspension in N/m
	const float rearSuspensionStiffness = 200000.0f;
	const float momentOfIntertiaX = 2000.0f; // moment of intertia about the X axis in kg m^2
	const float momentOfIntertiaY = 3000.0f;
	const float momentOfIntertiaZ = 5000.0f;
	const float frontArea = 2.2f; // frontal area in m^2
	const float dragCoefficient = 0.39f; // the drag coefficient used for calculating the air resistance
	const float rollResistanceCoefficient = 0.012f; // the rolling resistance coefficient used for calculating the roll resistance force
	const float airDensity = 1.225f; // density of the air in kg/m^3
	const float g = 9.81f; // gravity

	// Tire constants
	const float tireRadius = 0.0f;
	const float tireInertia = 0.0f;
	const float relaxationLengthCoefficient = 0.5f;
	const float threadStiffness = 30.0f; // N/m
	const float casterOffset = 0.03f; // m
	const float contactPatchLength = 0.1f; // m
	const float slidingFrictionConstant = 0.7f; // for dry asphalt

	float CalculateLoadChangeFromLateralForce(float lateralForce, bool forFrontAxel);
	float CalculateWheelLoadRatio();
};
