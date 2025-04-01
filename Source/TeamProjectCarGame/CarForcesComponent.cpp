// Fill out your copyright notice in the Description page of Project Settings.


#include "CarForcesComponent.h"

#include "DSP/BufferDiagnostics.h"
#include "Math/UnrealMathUtility.h"

// Sets default values for this component's properties
UCarForcesComponent::UCarForcesComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	carVelocity = FVector::ZeroVector;
	carAcceleration = FVector::ZeroVector;
	carAngularVelocity = FVector::ZeroVector;
	carAngularAcceleration = FVector::ZeroVector;
	
	totalWheelLoadToCarWeightRatio = CalculateWheelLoadRatio();
	longitudinalAcceleration = 0.0f;
	lateralAcceleration = 0.0f;

	clutchInput = 0.0f;
	throttleInput = 0.0f;

	tireFR = FTireInfo(staticWheelLoads.FrontRight, frontWheelOffset, frontTrackWidth, tireRadius, threadStiffness, contactPatchLength, casterOffset, relaxationLengthCoefficient, true);
	tireFL = FTireInfo(staticWheelLoads.FrontLeft, frontWheelOffset, frontTrackWidth, tireRadius, threadStiffness, contactPatchLength, casterOffset, relaxationLengthCoefficient, true);
	tireRR = FTireInfo(staticWheelLoads.RearLeft, rearWheelOffset, rearTrackWidth, tireRadius, threadStiffness, contactPatchLength, casterOffset, relaxationLengthCoefficient, false);
	tireRL = FTireInfo(staticWheelLoads.RearLeft, rearWheelOffset, rearTrackWidth, tireRadius, threadStiffness, contactPatchLength, casterOffset, relaxationLengthCoefficient, false);
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

// calculates the loads on each wheel of the car and returns a struct containing the results
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

// calculates the ratio of the sum of the wheel loads to the mass (should always return 1.0)
float UCarForcesComponent::CalculateWheelLoadRatio()
{
	return (tireFR.load + tireFL.load + tireRR.load + tireRL.load)/mass;
}


float UCarForcesComponent::CalculateLoadChangeFromCorneringForce(float lateralForce, bool forFrontAxel)
{
	float trackWidth = (forFrontAxel) ? frontTrackWidth : rearTrackWidth;
	float rollCentreToCentreOfMass = centreOfMassHeight - rollCentreHeight;
	float rollAngleNumerator =  lateralForce * rollCentreToCentreOfMass;
	float rollAngle = rollAngleNumerator / (frontSuspensionStiffness + rearSuspensionStiffness - g * mass * rollCentreToCentreOfMass);
	float loadChange = lateralForce*rollCentreHeight/trackWidth + (frontSuspensionStiffness + rearSuspensionStiffness)*rollAngle/trackWidth;

	return loadChange/g;
}

void UCarForcesComponent::CalculateWheelLoads(float bankAngle, float gradientAngle, float accel)
{
	float frontLoadChange = CalculateLoadChangeFromCorneringForce(mass*carAcceleration.Y, true);
	float rearLoadChange = CalculateLoadChangeFromCorneringForce(mass*carAcceleration.Y, false);
	//                                                   load change from turning                                        load change from banking           load change from a gradient       load change from forward acceleration
	tireFR.load = staticWheelLoads.FrontRight +frontLoadChange+ staticWheelLoads.FrontRight * centreOfMassHeight * ((2.0f*bankAngle/frontTrackWidth) - (gradientAngle/frontWheelOffset) - (accel/(g*frontWheelOffset)));
	tireFL.load = staticWheelLoads.FrontLeft -frontLoadChange+ staticWheelLoads.FrontLeft * centreOfMassHeight * (-2.0f*bankAngle/frontTrackWidth - gradientAngle/frontWheelOffset - accel/(g*frontWheelOffset));
	tireRR.load = staticWheelLoads.RearRight +rearLoadChange+ staticWheelLoads.RearRight * centreOfMassHeight * (2.0f * bankAngle/rearTrackWidth + gradientAngle/rearWheelOffset + accel/(g*rearWheelOffset));
	tireRL.load = staticWheelLoads.RearLeft -rearLoadChange+ staticWheelLoads.RearLeft * centreOfMassHeight * (-2.0f * bankAngle/rearTrackWidth + gradientAngle/rearWheelOffset + accel/(g*rearWheelOffset));

	totalWheelLoadToCarWeightRatio = CalculateWheelLoadRatio();
}

void UCarForcesComponent::CalculateWheelsLocalVelocities()
{
	tireFR.CalculateLocalVelocity(-1.0f, 1.0f, carVelocity, carAngularVelocity.Z);
	tireFL.CalculateLocalVelocity(1.0f, 1.0f, carVelocity, carAngularVelocity.Z);
	tireRR.CalculateLocalVelocity(-1.0f, -1.0f, carVelocity, carAngularVelocity.Z);
	tireRL.CalculateLocalVelocity(1.0f, -1.0f, carVelocity, carAngularVelocity.Z);
}


// calculates the drag force. using F = 1/2 * ρ * v^2 * A
// calculates the roll resistance using F = f * m * g. Min(1, velocity) is used to remove the rolling resistance when the car is stationary
// * -Sign(velocity) makes sure the drag and roll resistance are in the opposite direction
// calculates the resistance due to the car's weight when on a slope using F = m * g * sin(θ)
void UCarForcesComponent::CalculateWheelForces(bool carIsGrounded, bool carIsWallDragging, bool frontWallCollision, bool backWallCollision)
{
	float drag = 0.5f * airDensity * dragCoefficient * frontArea * carVelocity.X * carVelocity.X * -FMath::Sign(carVelocity.X);
	float collisionForce = ((frontWallCollision && carVelocity.X > 0) || (backWallCollision && carVelocity.X < 0))? 0.5f * mass * carVelocity.X * carVelocity.X * -FMath::Sign(carVelocity.X) / 0.5f : 0.0f;

	if (!carIsGrounded)
	{
		tireFR.localLongitudinalForce = drag + collisionForce/4.0f;
		tireFL.localLongitudinalForce = drag + collisionForce/4.0f;
		tireRR.localLongitudinalForce = drag + collisionForce/4.0f;
		tireRL.localLongitudinalForce = drag + collisionForce/4.0f;
		return;
	}
	
	float rollResistance = rollResistanceCoefficient * mass * g * FMath::Min(1.0f, FMath::Abs(carVelocity.X)) * -FMath::Sign(carVelocity.X);
	float slope = - mass * g * FMath::Sin(gradient);
	float wallDrag = (carIsWallDragging)? FMath::Abs(carVelocity.X) * -FMath::Sign(carVelocity.X) * 500.0f: 0.0f;
	float intermidiate = ((drag + slope + rollResistance + wallDrag + collisionForce)/4.0f);

	float brakeValue = (engineInfo.currentGear == -1)? throttleInput : brakeInput;
	tireFR.localLongitudinalForce = intermidiate + (tireFR.brake.CalculateBrakingTorque(brakeValue, tireFR.angularVelocity, engineInfo.currentGear) / tireFR.radius);
	tireFL.localLongitudinalForce = intermidiate + (tireFL.brake.CalculateBrakingTorque(brakeValue, tireFL.angularVelocity, engineInfo.currentGear) / tireFL.radius);
	tireRR.localLongitudinalForce = intermidiate + ((engineInfo.drivingTorquePerWheel + tireRR.brake.CalculateBrakingTorque(brakeValue, tireRR.angularVelocity, engineInfo.currentGear)) / tireRR.radius);
	tireRL.localLongitudinalForce = intermidiate + ((engineInfo.drivingTorquePerWheel + tireRL.brake.CalculateBrakingTorque(brakeValue, tireRL.angularVelocity, engineInfo.currentGear)) / tireRL.radius);
}

// The rotational force can be used to get the angular acceleration (cars local yaw acceleration) using F = Iα (I: moment of inertia in the Z axis, α: angular acceleration)
FCarForces UCarForcesComponent::CalculateCarForces()
{
	FCarForces carForces;
	float totalLongitudinalForce = 0.0f;
	float totalLateralForce = 0.0f;
	float totalRotationalForce = 0.0f;
	
	totalLongitudinalForce += tireFR.localLongitudinalForce;
	totalLongitudinalForce += tireFL.localLongitudinalForce;
	totalLongitudinalForce += tireRR.localLongitudinalForce;
	totalLongitudinalForce += tireRL.localLongitudinalForce;
	
	totalLateralForce += tireFR.localLateralForce;
	totalLateralForce += tireFL.localLateralForce;
	totalLateralForce += tireRR.localLateralForce;
	totalLateralForce += tireRL.localLateralForce;

	carForces.longitudinalForce = totalLongitudinalForce;
	carForces.lateralForce = totalLateralForce;
	carForces.angularForce = totalRotationalForce;

	return carForces;
}

void UCarForcesComponent::ApplyAllAccelerations(float deltaTime)
{
	carVelocity += carAcceleration * deltaTime;
	carSpeed = carVelocity.X * 2.23694f;
	carAngularVelocity += carAngularAcceleration * deltaTime;
	
	CheckForAdditionalBraking();
	
	tireFR.angularVelocity = carVelocity.X/ tireFR.radius;
	tireFL.angularVelocity = carVelocity.X/ tireFL.radius;
	tireRR.angularVelocity = carVelocity.X/ tireRR.radius;
	tireRL.angularVelocity = carVelocity.X/ tireRL.radius;
}

void UCarForcesComponent::PerformSimulationFrame(float deltaTime, bool carIsGrounded, bool carIsWallDragging, bool frontWallCollision, bool backWallCollision)
{
	CheckForGearShift();
	
	engineInfo.CalculateEngineTorqueRange();
	engineInfo.CalculateEngineVelocity(tireRR.angularVelocity, tireRL.angularVelocity, clutchInput, throttleInput);
	engineInfo.CalculateEngineTorque(clutchInput, (engineInfo.currentGear == -1)? brakeInput : throttleInput, carVelocity.X);
	
	CalculateWheelLoads(0.0f, 0.0f, carAcceleration.X);

	tireFR.CalculateLocalVelocity(-1.0f, 1.0f, carVelocity, carAngularVelocity.Z);
	tireFL.CalculateLocalVelocity(1.0f, 1.0f, carVelocity, carAngularVelocity.Z);
	tireRR.CalculateLocalVelocity(-1.0f, -1.0f, carVelocity, carAngularVelocity.Z);
	tireRL.CalculateLocalVelocity(1.0f, -1.0f, carVelocity, carAngularVelocity.Z);

	CalculateWheelForces(carIsGrounded, carIsWallDragging, frontWallCollision, backWallCollision);
	FCarForces forces = CalculateCarForces();
	carAcceleration = FVector(forces.longitudinalForce/mass, forces.lateralForce/mass, 0.0f);
	carAngularAcceleration = FVector(0.0f, 0.0f, forces.angularForce/mass);

	CalculateCarAngularVelocity();
	
	ApplyAllAccelerations(deltaTime);
}

void UCarForcesComponent::CheckForGearShift()
{
	int gear = engineInfo.currentGear;

	if (FMath::Abs(carVelocity.X) < 0.005f)
	{
		engineInfo.SwapGears(0);
	}

	if (gear == 0 && FMath::Abs(carVelocity.X) < 0.005f)
	{
		engineInfo.SwapGears((throttleInput > 0.0f)? 1 : (brakeInput > 0.0f)? -1 : 0);
	}

	if (engineInfo.currentGear == -1 || engineInfo.currentGear == 0) return;

	int gearUp = FMath::Clamp(engineInfo.currentGear + 1, 0, 5);
	int gearDown = FMath::Clamp(engineInfo.currentGear - 1, 1, 5);
	if (engineInfo.engineRPM > 8500.0f) engineInfo.SwapGears(gearUp);
	else if (engineInfo.engineRPM < 5000.0f) engineInfo.SwapGears(gearDown);
}

void UCarForcesComponent::CalculateCarAngularVelocity()
{
	float speed = carVelocity.X;
	float delta = tireFR.delta;

	float turningRadius = wheelSeparation/FMath::Tan(FMath::DegreesToRadians(delta));
	carAngularVelocity = FVector(0.0f, 0.0f, speed/turningRadius);
}

void UCarForcesComponent::CheckForAdditionalBraking()
{
	if (throttleInput > 0.0f || brakeInput > 0.0f) return;

	if (FMath::Abs(carSpeed) < 1.0f) ResetCarSpeed();
}

void UCarForcesComponent::ResetCarSpeed()
{
	carVelocity.X = 0.0f;
	engineInfo.SwapGears(0);
}















