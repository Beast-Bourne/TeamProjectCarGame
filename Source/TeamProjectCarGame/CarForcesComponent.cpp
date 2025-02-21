// Fill out your copyright notice in the Description page of Project Settings.


#include "CarForcesComponent.h"
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

	tireFR = FTireInfo(staticWheelLoads.FrontRight, frontWheelOffset, frontTrackWidth, tireRadius, threadStiffness, contactPatchLength, casterOffset, relaxationLengthCoefficient);
	tireFL = FTireInfo(staticWheelLoads.FrontLeft, frontWheelOffset, frontTrackWidth, tireRadius, threadStiffness, contactPatchLength, casterOffset, relaxationLengthCoefficient);
	tireRR = FTireInfo(staticWheelLoads.RearLeft, rearWheelOffset, rearTrackWidth, tireRadius, threadStiffness, contactPatchLength, casterOffset, relaxationLengthCoefficient);
	tireRL = FTireInfo(staticWheelLoads.RearLeft, rearWheelOffset, rearTrackWidth, tireRadius, threadStiffness, contactPatchLength, casterOffset, relaxationLengthCoefficient);
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

	PerformSimulationFrame(DeltaTime);
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
	tireFR.UpdateMemberVariables(0.0f, 0.0f, 1.0f);
	tireFL.CalculateLocalVelocity(1.0f, 1.0f, carVelocity, carAngularVelocity.Z);
	tireFL.UpdateMemberVariables(0.0f, 0.0f, 1.0f);
	tireRR.CalculateLocalVelocity(-1.0f, -1.0f, carVelocity, carAngularVelocity.Z);
	tireRR.UpdateMemberVariables(0.0f, 0.0f, -1.0f);
	tireRL.CalculateLocalVelocity(1.0f, -1.0f, carVelocity, carAngularVelocity.Z);
	tireRL.UpdateMemberVariables(0.0f, 0.0f, -1.0f);
}


// calculates the drag force. using F = 1/2 * ρ * v^2 * A
// calculates the roll resistance using F = f * m * g. Min(1, velocity) is used to remove the rolling resistance when the car is stationary
// * -Sign(velocity) makes sure the drag and roll resistance are in the opposite direction
// calculates the resistance due to the car's weight when on a slope using F = m * g * sin(θ)
void UCarForcesComponent::CalculateWheelForces()
{
	float drag = 0.5f * airDensity * dragCoefficient * frontArea * carVelocity.X * carVelocity.X * -FMath::Sign(carVelocity.X);
	float rollResistance = rollResistanceCoefficient * mass * g * FMath::Min(1.0f, FMath::Abs(carVelocity.X)) * -FMath::Sign(carVelocity.X);
	float slope = - mass * g * FMath::Sin(gradient);
	float intermidiate = (drag + slope + rollResistance)/4.0f;

	tireFR.localLongitudinalForce = intermidiate;
	tireFL.localLongitudinalForce = intermidiate;
	tireRR.localLongitudinalForce = intermidiate + (engineInfo.drivingTorquePerWheel * tireRR.radius);
	tireRL.localLongitudinalForce = intermidiate + (engineInfo.drivingTorquePerWheel * tireRL.radius);
}

// The rotational force can be used to get the angular acceleration (cars local yaw acceleration) using F = Iα (I: moment of inertia in the Z axis, α: angular acceleration)
FCarForces UCarForcesComponent::CalculateCarForces()
{
	FCarForces carForces;
	float totalLongitudinalForce = 0.0f;
	float totalLateralForce = 0.0f;
	float totalRotationalForce = 0.0f;
	
	totalLongitudinalForce += tireFR.ReturnLongitudinalForceForCar();
	totalLongitudinalForce += tireFL.ReturnLongitudinalForceForCar();
	totalLongitudinalForce += tireRR.ReturnLongitudinalForceForCar();
	totalLongitudinalForce += tireRL.ReturnLongitudinalForceForCar();
	
	totalLateralForce += tireFR.ReturnLateralForceForCar();
	totalLateralForce += tireFL.ReturnLateralForceForCar();
	totalLateralForce += tireRR.ReturnLateralForceForCar();
	totalLateralForce += tireRL.ReturnLateralForceForCar();

	//totalRotationalForce += tireFR.ReturnLateralForceForCar() * frontWheelOffset - tireFR.ReturnLongitudinalForceForCar() * 0.5f * frontTrackWidth;
	//totalRotationalForce += tireFL.ReturnLateralForceForCar() * frontWheelOffset + tireFL.ReturnLongitudinalForceForCar() * 0.5f * frontTrackWidth;
	//totalRotationalForce += tireRR.ReturnLateralForceForCar() * -rearWheelOffset - tireRR.ReturnLongitudinalForceForCar() * 0.5f * rearTrackWidth;
	//totalRotationalForce += tireRL.ReturnLateralForceForCar() * -rearWheelOffset + tireRL.ReturnLongitudinalForceForCar() * 0.5f * rearTrackWidth;

	carForces.longitudinalForce = totalLongitudinalForce;
	carForces.lateralForce = totalLateralForce;
	carForces.angularForce = totalRotationalForce;

	return carForces;
}

void UCarForcesComponent::ApplyAllAccelerations(float deltaTime)
{
	carVelocity += carAcceleration * deltaTime;
	carAngularVelocity += carAngularAcceleration * deltaTime;

	tireFR.angularVelocity = carVelocity.X/ tireFR.radius;
	tireFL.angularVelocity = carVelocity.X/ tireFL.radius;
	tireRR.angularVelocity = carVelocity.X/ tireRR.radius;
	tireRL.angularVelocity = carVelocity.X/ tireRL.radius;
}

void UCarForcesComponent::PerformSimulationFrame(float deltaTime)
{
	engineInfo.CalculateEngineTorqueRange();
	engineInfo.CalculateEngineVelocity(tireRR.angularVelocity, tireRL.angularVelocity, clutchInput, throttleInput);
	engineInfo.CalculateEngineTorque(clutchInput, throttleInput, carVelocity.X);
	
	CalculateWheelLoads(0.0f, 0.0f, carAcceleration.X);

	tireFR.CalculateLocalVelocity(-1.0f, 1.0f, carVelocity, carAngularVelocity.Z);
	tireFL.CalculateLocalVelocity(1.0f, 1.0f, carVelocity, carAngularVelocity.Z);
	tireRR.CalculateLocalVelocity(-1.0f, -1.0f, carVelocity, carAngularVelocity.Z);
	tireRL.CalculateLocalVelocity(1.0f, -1.0f, carVelocity, carAngularVelocity.Z);
	
	tireFR.UpdateMemberVariables(0.0f, 0.0f, 1.0f);
	tireFL.UpdateMemberVariables(0.0f, 0.0f, 1.0f);
	tireRR.UpdateMemberVariables(engineInfo.drivingTorquePerWheel, 0.0f, -1.0f);
	tireRL.UpdateMemberVariables(engineInfo.drivingTorquePerWheel, 0.0f, -1.0f);

	CalculateWheelForces();
	FCarForces forces = CalculateCarForces();
	carAcceleration = FVector(forces.longitudinalForce/mass, forces.lateralForce/mass, 0.0f);
	carAngularAcceleration = FVector(0.0f, 0.0f, forces.angularForce/mass);

	ApplyAllAccelerations(deltaTime);
}

void UCarForcesComponent::ChangeGear(int gear)
{
	engineInfo.SwapGears(gear);
}













