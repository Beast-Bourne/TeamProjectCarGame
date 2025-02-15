// Fill out your copyright notice in the Description page of Project Settings.


#include "CarForcesComponent.h"
#include "Math/UnrealMathUtility.h"

// Sets default values for this component's properties
UCarForcesComponent::UCarForcesComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	carVelocity = FVector(1.0f, 0.0f, 0.0f);
	carAcceleration = FVector::ZeroVector;
	carAngularVelocity = FVector::ZeroVector;
	carAngularAcceleration = FVector::ZeroVector;
	
	wheelLoads = CalculateStaticWheelLoads();
	totalWheelLoadToCarWeightRatio = CalculateWheelLoadRatio();
	longitudinalAcceleration = 0.0f;
	lateralAcceleration = 0.0f;

	clutchInput = 0.75f;
	throttleInput = 0.8f;

	InitialiseTireArray();
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

void UCarForcesComponent::InitialiseTireArray()
{
	FTireInfo FRTireInfo(staticWheelLoads.FrontRight, frontWheelOffset, frontTrackWidth, tireRadius, threadStiffness, contactPatchLength, casterOffset, relaxationLengthCoefficient);
	FTireInfo FLTireInfo(staticWheelLoads.FrontLeft, frontWheelOffset, frontTrackWidth, tireRadius, threadStiffness, contactPatchLength, casterOffset, relaxationLengthCoefficient);
	FTireInfo RRTireInfo(staticWheelLoads.RearLeft, rearWheelOffset, rearTrackWidth, tireRadius, threadStiffness, contactPatchLength, casterOffset, relaxationLengthCoefficient);
	FTireInfo RLTireInfo(staticWheelLoads.RearLeft, rearWheelOffset, rearTrackWidth, tireRadius, threadStiffness, contactPatchLength, casterOffset, relaxationLengthCoefficient);
	
	TireMap.Add(EWheelPosition::FrontRight, FRTireInfo);
	TireMap.Add(EWheelPosition::FrontLeft, FLTireInfo);
	TireMap.Add(EWheelPosition::RearRight, RRTireInfo);
	TireMap.Add(EWheelPosition::RearLeft, RLTireInfo);
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


float UCarForcesComponent::CalculateLoadChangeFromCorneringForce(float lateralForce, bool forFrontAxel)
{
	float trackWidth = (forFrontAxel) ? frontTrackWidth : rearTrackWidth;
	float rollCentreToCentreOfMass = centreOfMassHeight - rollCentreHeight;
	float rollAngleNumerator =  lateralForce * rollCentreToCentreOfMass;
	float rollAngle = rollAngleNumerator / (frontSuspensionStiffness + rearSuspensionStiffness - g * mass * rollCentreToCentreOfMass);
	float loadChange = lateralForce*rollCentreHeight/trackWidth + (frontSuspensionStiffness + rearSuspensionStiffness)*rollAngle/trackWidth;

	return loadChange/g;
}

FWheelLoads UCarForcesComponent::CalculateWheelLoads(float bankAngle, float gradientAngle, float accel)
{
	float frontLoadChange = CalculateLoadChangeFromCorneringForce(mass*carAcceleration.Y, true);
	float rearLoadChange = CalculateLoadChangeFromCorneringForce(mass*carAcceleration.Y, false);
	//                                                   load change from turning                                        load change from banking           load change from a gradient       load change from forward acceleration
	wheelLoads.FrontRight = staticWheelLoads.FrontRight +frontLoadChange+ staticWheelLoads.FrontRight * centreOfMassHeight * ((2.0f*bankAngle/frontTrackWidth) - (gradientAngle/frontWheelOffset) - (accel/(g*frontWheelOffset)));
	wheelLoads.FrontLeft = staticWheelLoads.FrontLeft -frontLoadChange+ staticWheelLoads.FrontLeft * centreOfMassHeight * (-2.0f*bankAngle/frontTrackWidth - gradientAngle/frontWheelOffset - accel/(g*frontWheelOffset));
	wheelLoads.RearRight = staticWheelLoads.RearRight +rearLoadChange+ staticWheelLoads.RearRight * centreOfMassHeight * (2.0f * bankAngle/rearTrackWidth + gradientAngle/rearWheelOffset + accel/(g*rearWheelOffset));
	wheelLoads.RearLeft = staticWheelLoads.RearLeft -rearLoadChange+ staticWheelLoads.RearLeft * centreOfMassHeight * (-2.0f * bankAngle/rearTrackWidth + gradientAngle/rearWheelOffset + accel/(g*rearWheelOffset));

	totalWheelLoadToCarWeightRatio = CalculateWheelLoadRatio();
	return wheelLoads;
}

void UCarForcesComponent::CalculateWheelsLocalVelocities()
{
	FTireInfo &tireFR = TireMap[EWheelPosition::FrontRight];
	FTireInfo &tireFL = TireMap[EWheelPosition::FrontLeft];
	FTireInfo &tireRR = TireMap[EWheelPosition::RearRight];
	FTireInfo &tireRL = TireMap[EWheelPosition::RearLeft];

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
float UCarForcesComponent::CalculateResistanceForce()
{
	float drag = 0.5f * airDensity * dragCoefficient * frontArea * velocity * velocity * -FMath::Sign(velocity);
	float rollResistance = rollResistanceCoefficient * mass * g * FMath::Min(1, velocity) * -FMath::Sign(velocity);
	float slope = - mass * g * FMath::Sin(gradient);
	
	return drag + rollResistance + slope;
}

// The rotational force can be used to get the angular acceleration (cars local yaw acceleration) using F = Iα (I: moment of inertia in the Z axis, α: angular acceleration)
FCarForces UCarForcesComponent::CalculateCarForces()
{
	FCarForces carForces;
	float totalLongitudinalForce = 0.0f;
	float totalLateralForce = 0.0f;
	float totalRotationalForce = 0.0f;

	totalLongitudinalForce += TireMap[EWheelPosition::FrontRight].ReturnLongitudinalForceForCar();
	totalLongitudinalForce += TireMap[EWheelPosition::FrontLeft].ReturnLongitudinalForceForCar();
	totalLongitudinalForce += TireMap[EWheelPosition::RearRight].ReturnLongitudinalForceForCar();
	totalLongitudinalForce += TireMap[EWheelPosition::RearLeft].ReturnLongitudinalForceForCar();
	totalLongitudinalForce += CalculateResistanceForce();
	
	totalLateralForce += TireMap[EWheelPosition::FrontRight].ReturnLateralForceForCar();
	totalLateralForce += TireMap[EWheelPosition::FrontLeft].ReturnLateralForceForCar();
	totalLateralForce += TireMap[EWheelPosition::RearRight].ReturnLateralForceForCar();
	totalLateralForce += TireMap[EWheelPosition::RearLeft].ReturnLateralForceForCar();

	totalRotationalForce += TireMap[EWheelPosition::FrontRight].ReturnLateralForceForCar() * frontWheelOffset - TireMap[EWheelPosition::FrontRight].ReturnLongitudinalForceForCar() * 0.5f * frontTrackWidth;
	totalRotationalForce += TireMap[EWheelPosition::FrontLeft].ReturnLateralForceForCar() * frontWheelOffset + TireMap[EWheelPosition::FrontLeft].ReturnLongitudinalForceForCar() * 0.5f * frontTrackWidth;
	totalRotationalForce += TireMap[EWheelPosition::RearRight].ReturnLateralForceForCar() * -rearWheelOffset - TireMap[EWheelPosition::RearRight].ReturnLongitudinalForceForCar() * 0.5f * rearTrackWidth;
	totalRotationalForce += TireMap[EWheelPosition::RearLeft].ReturnLateralForceForCar() * -rearWheelOffset + TireMap[EWheelPosition::RearLeft].ReturnLongitudinalForceForCar() * 0.5f * rearTrackWidth;

	carForces.longitudinalForce = totalLongitudinalForce;
	carForces.lateralForce = totalLateralForce;
	carForces.angularForce = totalRotationalForce;

	UE_LOG(LogTemp, Log, TEXT("longitudinal force on car: %f"), totalLongitudinalForce);

	return carForces;
}

void UCarForcesComponent::ApplyAllAccelerations(float deltaTime)
{
	carVelocity += carAcceleration * deltaTime;
	carAngularVelocity += carAngularAcceleration * deltaTime;
}

void UCarForcesComponent::PerformSimulationFrame(float deltaTime)
{
	engineInfo.CalculateEngineVelocity(TireMap[EWheelPosition::RearRight].angularVelocity, TireMap[EWheelPosition::RearLeft].angularVelocity, clutchInput, throttleInput);
	engineInfo.CalculateEngineTorque(clutchInput, throttleInput, velocity);

	FTireInfo &tireFR = TireMap[EWheelPosition::FrontRight];
	FTireInfo &tireFL = TireMap[EWheelPosition::FrontLeft];
	FTireInfo &tireRR = TireMap[EWheelPosition::RearRight];
	FTireInfo &tireRL = TireMap[EWheelPosition::RearLeft];
	
	tireFR.CalculateAngularAccel(0.0f, 0.0f);
	tireFL.CalculateAngularAccel(0.0f, 0.0f);
	tireRR.CalculateAngularAccel(engineInfo.drivingTorquePerWheel, 0.0f);
	tireRL.CalculateAngularAccel(engineInfo.drivingTorquePerWheel, 0.0f);

	FWheelLoads loads = CalculateWheelLoads(0.0f, 0.0f, carAcceleration.X);
	tireFR.load = loads.FrontRight;
	tireFL.load = loads.FrontLeft;
	tireRR.load = loads.RearRight;
	tireRL.load = loads.RearLeft;

	tireFR.CalculateLocalVelocity(-1.0f, 1.0f, carVelocity, carAngularVelocity.Z);
	tireFL.CalculateLocalVelocity(1.0f, 1.0f, carVelocity, carAngularVelocity.Z);
	tireRR.CalculateLocalVelocity(-1.0f, -1.0f, carVelocity, carAngularVelocity.Z);
	tireRL.CalculateLocalVelocity(1.0f, -1.0f, carVelocity, carAngularVelocity.Z);
	
	tireFR.UpdateMemberVariables(0.0f, 0.0f, 1.0f);
	tireFL.UpdateMemberVariables(0.0f, 0.0f, 1.0f);
	tireRR.UpdateMemberVariables(engineInfo.drivingTorquePerWheel, 0.0f, -1.0f);
	tireRL.UpdateMemberVariables(engineInfo.drivingTorquePerWheel, 0.0f, -1.0f);

	FCarForces forces = CalculateCarForces();
	carAcceleration = FVector(forces.longitudinalForce/mass, forces.lateralForce/mass, 0.0f);
	carAngularAcceleration = FVector(0.0f, 0.0f, forces.angularForce/mass);

	ApplyAllAccelerations(deltaTime);
}












