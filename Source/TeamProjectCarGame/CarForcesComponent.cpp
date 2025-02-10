// Fill out your copyright notice in the Description page of Project Settings.


#include "CarForcesComponent.h"
#include "Math/UnrealMathUtility.h"

// Sets default values for this component's properties
UCarForcesComponent::UCarForcesComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	wheelLoads = CalculateStaticWheelLoads();
	totalWheelLoadToCarWeightRatio = CalculateWheelLoadRatio();
	longitudinalAcceleration = 0.0f;
	lateralAcceleration = 0.0f;
}


// Called when the game starts
void UCarForcesComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UCarForcesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FWheelLoads UCarForcesComponent::CalculateStaticWheelLoads()
{
	float frontLoad = mass * frontWheelOffset/wheelSeparation;
	float rearLoad = mass * rearWheelOffset/wheelSeparation;

	FWheelLoads loads;
	loads.FrontLeft = frontLoad/2.0f;
	loads.RearLeft = rearLoad/2.0f;
	loads.FrontRight = frontLoad/2.0f;
	loads.RearRight = rearLoad/2.0f;
	
	return loads;
}

float UCarForcesComponent::CalculateWheelLoadRatio()
{
	return (wheelLoads.FrontLeft + wheelLoads.RearLeft + wheelLoads.FrontRight + wheelLoads.RearRight)/mass;
}


float UCarForcesComponent::CalculateLoadChangeFromLateralForce(float lateralForce, bool forFrontAxel)
{
	float trackWidth = (forFrontAxel) ? frontTrackWidth : rearTrackWidth;
	float rollCentreToCentreOfMass = centreOfMassHeight - rollCentreHeight;
	float rollAngleNumerator =  lateralForce * rollCentreToCentreOfMass;
	float rollAngle = rollAngleNumerator / (frontSuspensionStiffness + rearSuspensionStiffness - g * mass * rollCentreToCentreOfMass);
	float loadChange = lateralForce*rollCentreHeight/trackWidth + (frontSuspensionStiffness + rearSuspensionStiffness)*rollAngle/trackWidth;

	return loadChange/g;
}

FWheelLoads UCarForcesComponent::CalculateSlopedWheelLoads(float bankAngle, float gradientAngle, float accel)
{
	float frontLoadChange = CalculateLoadChangeFromLateralForce(mass*lateralAcceleration, true);
	float rearLoadChange = CalculateLoadChangeFromLateralForce(mass*lateralAcceleration, false);
	//                                                   load change from turning                                        load change from banking           load change from a gradient       load change from forward acceleration
	wheelLoads.FrontRight = staticWheelLoads.FrontRight +frontLoadChange+ staticWheelLoads.FrontRight * centreOfMassHeight * ((2.0f*bankAngle/frontTrackWidth) - (gradientAngle/frontWheelOffset) - (accel/(g*frontWheelOffset)));
	wheelLoads.FrontLeft = staticWheelLoads.FrontLeft -frontLoadChange+ staticWheelLoads.FrontLeft * centreOfMassHeight * (-2.0f*bankAngle/frontTrackWidth - gradientAngle/frontWheelOffset - accel/(g*frontWheelOffset));
	wheelLoads.RearRight = staticWheelLoads.RearRight +rearLoadChange+ staticWheelLoads.RearRight * centreOfMassHeight * (2.0f * bankAngle/rearTrackWidth + gradientAngle/rearWheelOffset + accel/(g*rearWheelOffset));
	wheelLoads.RearLeft = staticWheelLoads.RearLeft -rearLoadChange+ staticWheelLoads.RearLeft * centreOfMassHeight * (-2.0f * bankAngle/rearTrackWidth + gradientAngle/rearWheelOffset + accel/(g*rearWheelOffset));

	totalWheelLoadToCarWeightRatio = CalculateWheelLoadRatio();
	return wheelLoads;
}

// calculates the drag force. using F = 1/2 * ρ * v^2 * A
// calculates the roll resistance using F = f * m * g. Min(1, velocity) is used to remove the rolling resistance when the car is stationary
// * -Sign(velocity) makes sure the drag and roll resistance are in the opposite direction
// calculates the resistance due to the car's weight when on a slope using F = m * g * sin(θ)
float UCarForcesComponent::calculateResistanceForce()
{
	float drag = 0.5f * airDensity * dragCoefficient * frontArea * velocity * velocity * -FMath::Sign(velocity);
	float rollResistance = rollResistanceCoefficient * mass * g * FMath::Min(1, velocity) * -FMath::Sign(velocity);
	float slope = - mass * g * FMath::Sin(gradient);
	
	return drag + rollResistance + slope;
}









