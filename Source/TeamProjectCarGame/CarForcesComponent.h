// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Math/UnrealMathUtility.h"
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

UENUM(BlueprintType)
enum class EWheelPosition : uint8
{
	FrontRight,
	FrontLeft,
	RearRight,
	RearLeft
};

USTRUCT(BlueprintType)
struct FTireInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FVector localVelocity;             // velocity of the tire relative to the ground
	UPROPERTY(BlueprintReadOnly)
	float angularVelocity;             // angular velocity of the tire
	UPROPERTY(BlueprintReadOnly)
	float slipVelocity;                // velocity of the tire in a no-slip condition
	UPROPERTY(BlueprintReadOnly)
	float longitudinalSlipFactor;      // slip of the tire in the longitudinal direction
	UPROPERTY(BlueprintReadOnly)
	float lateralSlipFactor;           // slip of the tire in the lateral direction
	UPROPERTY(BlueprintReadOnly)
	float combinedSlipFactor;          // slip factor used for determining conditions for calculating slip forces
	UPROPERTY(BlueprintReadOnly)
	float slipConditionFactor;         // factor used for checking if the tire is working in linear or sliding conditions
	UPROPERTY(BlueprintReadOnly)
	float threadStiffness;
	UPROPERTY(BlueprintReadOnly)
	float staticFrictionCoefficient;
	float slidingFrictionCoefficient;
	float xSlipFriction;
	float ySlipFriction;
	float patchLength;
	float selfAligningTorque;
	float casterOffset;
	float relaxationConstant;

	float xSlipForceWithRelaxation;
	float ySlipForceWithRelaxation;
	float selfAligningTorqueWithRelaxation;
	
	
	float load; // normal force acting on the tire in kg
	float localLongitudinalForce;
	float localLateralForce;
	float localSelfAligningTorque;
	float delta; // The angle between the tires local forward direction and the cars forward direction
	float offsetFromCentreOfMass; // offset (in x-axis) between the wheel and the car's CoM
	float trackWidth; // width of the axel
	float radius; // tire's radius

	float momentOfInertia = 2.2f;
	float angularAcceleration = 0.0f;

	// default constructor
	FTireInfo()
	{
		localVelocity = FVector::ZeroVector;
		angularVelocity = 0;
		slipVelocity = 0;
		longitudinalSlipFactor = 0;
		lateralSlipFactor = 0;
		combinedSlipFactor = 0;
		slipConditionFactor = 0;
		threadStiffness = 0;
		staticFrictionCoefficient = 0;
		slidingFrictionCoefficient = 0;
		xSlipFriction = 0;
		ySlipFriction = 0;
		patchLength = 0;
		selfAligningTorque = 0;
		casterOffset = 0;
		relaxationConstant = 0;
		xSlipForceWithRelaxation = 0;
		ySlipForceWithRelaxation = 0;
		selfAligningTorqueWithRelaxation = 0;

		load = 0;
		localLongitudinalForce = 0;
		localLateralForce = 0;
		localSelfAligningTorque = 0;
		delta = 0;
		offsetFromCentreOfMass = 0;
		trackWidth = 0;
		radius = 0;
	}

	// main constructor
	FTireInfo(float load, float offset, float trackWidth, float radius, float threadStiffness, float patchLength, float casterOffset, float relaxationConstant)
	{
		localVelocity = FVector::ZeroVector;
		angularVelocity = 0.0f;
		slipVelocity = 0.0f;
		longitudinalSlipFactor = 0.0f;
		lateralSlipFactor = 0.0f;
		combinedSlipFactor = 0.0f;
		slipConditionFactor = 0.0f;
		staticFrictionCoefficient = 1.0f;
		slidingFrictionCoefficient = 1.0f;
		xSlipFriction = 0.0f;
		ySlipFriction = 0.0f;
		selfAligningTorque = 0.0f;
		xSlipForceWithRelaxation = 0.0f;
		ySlipForceWithRelaxation = 0.0f;
		selfAligningTorqueWithRelaxation = 0.0f;
		
		this->load = load;
		localLongitudinalForce = 0.0f;
		localLateralForce = 0.0f;
		localSelfAligningTorque = 0.0f;
		delta = 0.0f;
		this->offsetFromCentreOfMass = offset;
		this->trackWidth = trackWidth;
		this->radius = radius;
		this->threadStiffness = threadStiffness;
		this->patchLength = patchLength;
		this->casterOffset = casterOffset;
		this->relaxationConstant = relaxationConstant;
	}

	float ReturnLongitudinalForceForCar()
	{
		return localLongitudinalForce * FMath::Cos(delta) - localLateralForce * FMath::Sin(delta);
	}
	float ReturnLateralForceForCar()
	{
		return localLongitudinalForce * FMath::Sin(delta) + localLateralForce * FMath::Cos(delta);
	}

	// sets the local velocity of this tire to the cars velocity and corrects it for any angular velocity about the yaw of the car
	void CalculateLocalVelocity(float xMultiplier, float yMultiplier, const FVector &carVelocity, float carYawAngularVelocity)
	{
		float xValue = carVelocity.X + xMultiplier * carYawAngularVelocity * trackWidth * 0.5f;
		float yValue = offsetFromCentreOfMass * carYawAngularVelocity + yMultiplier * carVelocity.Y;
		float zValue = localVelocity.Z;
		localVelocity = FVector(xValue, yValue, zValue);
	}

	void UpdateMemberVariables(float drivingTorque, float brakingTorque, float slipMultiplier)
	{
		slipVelocity = angularVelocity * radius;
		longitudinalSlipFactor = (slipVelocity -localVelocity.X)/FMath::Max(slipVelocity, 1.0f);
		float slipFunction = (FMath::Tanh(10 * localVelocity.X -8.0f)+1.0f)/2.0f;
		lateralSlipFactor = slipFunction * slipMultiplier * (delta - localVelocity.Y/FMath::Max(slipVelocity, 1.0f));
		combinedSlipFactor = FMath::Sqrt(longitudinalSlipFactor * longitudinalSlipFactor + lateralSlipFactor * lateralSlipFactor);
		slipConditionFactor = threadStiffness/(3*staticFrictionCoefficient) * combinedSlipFactor;

		float slipFrictionFactor = CalculateSlipFrictionFactor();
		xSlipFriction = slipFrictionFactor * longitudinalSlipFactor;
		ySlipFriction = slipFrictionFactor * lateralSlipFactor;
		selfAligningTorque = (-patchLength*threadStiffness*lateralSlipFactor)/3.0f * FMath::Square(FMath::Min(0, slipConditionFactor-1.0f)) * (7*slipConditionFactor -1.0f) * load * 9.81f - (casterOffset * slipFrictionFactor * lateralSlipFactor);

		xSlipForceWithRelaxation = -FMath::Max(slipVelocity/relaxationConstant, 0.1f) * (localLongitudinalForce - xSlipFriction);
		ySlipForceWithRelaxation = -FMath::Max(slipVelocity/relaxationConstant, 0.1f) * (localLateralForce - ySlipFriction);
		selfAligningTorqueWithRelaxation = -FMath::Max(slipVelocity/relaxationConstant, 0.1f) * (localSelfAligningTorque - selfAligningTorque);
	}

	float CalculateSlipFrictionFactor()
	{
		if (slipConditionFactor < 1.0f)
		{
			return -threadStiffness * (-1.0f * slipConditionFactor - FMath::Pow(slipConditionFactor, 2.0f/3.0f)) * load * 9.81f;
		}

		float exponent = -0.01f * (slipConditionFactor -1.0f) * (slipConditionFactor -1.0f);
		float part1 = (slidingFrictionCoefficient + (1-slidingFrictionCoefficient) * FMath::Exp(exponent));
		float part2 = staticFrictionCoefficient * load * 9.81f / combinedSlipFactor;
		return part1 * part2;
	}

	void CalculateAngularAccel(float drivingTorque, float brakingTorque)
	{
		angularAcceleration = (drivingTorque - brakingTorque - localLongitudinalForce * radius)/momentOfInertia;
	}
};

USTRUCT(BlueprintType)
struct FCarForces
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float longitudinalForce;
	UPROPERTY(BlueprintReadOnly)
	float lateralForce;
	UPROPERTY(BlueprintReadOnly)
	float angularForce;

	FCarForces()
	{
		longitudinalForce = 0.0f;
		lateralForce = 0.0f;
		angularForce = 0.0f;
	}
};

USTRUCT(BlueprintType)
struct FEngineInfo
{
	GENERATED_BODY()

	float engineAngularVelocity = 0.0f;
	float engineRPM = 0.0f;
	float engineMaxTorque = 0.0f;
	float engineMinTorque = 0.0f;
	float currentTorque;
	float gearTransmissionRatio = 0.0f; // the ratio is 0 for neutral.
	int currentGear = 0;
	float finalGearRatio = 3.86f; // Hard coded for car being used (CHANGE LATER)
	float  totalTransmissionRatio = 0.0f;
	float engineMaxSpeed = 943.0f; // Hard coded for car being used (CHANGE LATER)
	float engineMinSpeed = 89.0f; // Hard coded for car being used (CHANGE LATER)

	float transmissionEfficiency = 0.0f;
	float drivingTorquePerWheel = 0.0f;

	void CalculateEngineTorqueRange()
	{
		engineMaxTorque = (2.1486f * FMath::Pow(10.0f, -6) * FMath::Pow(engineAngularVelocity, 3))
		- (3.7390514f * FMath::Pow(10.0f, -6.0f) * engineAngularVelocity * engineAngularVelocity)
		+ (1.8250297732f * engineAngularVelocity);

		engineMinTorque = (2.152813f * FMath::Pow(10.0f, -4) * engineAngularVelocity * engineAngularVelocity)
		- (0.2413794863f * engineAngularVelocity);
	}

	// hard coded gear ratio values (CHANGE THIS LATER)
	void SwapGears(int newGear)
	{
		currentGear = newGear;
		gearTransmissionRatio = (currentGear == 0)? 0.0f :
		(currentGear == 1)? 3.92f:
		(currentGear == 2)? 2.29f:
		(currentGear == 3)? 1.55f:
		(currentGear == 4)? 1.18f:
		(currentGear == 5)? 0.94f:
		0.79f;
	}

	void CalculateTotalGearRatio()
	{
		totalTransmissionRatio = (currentGear > 0) ? gearTransmissionRatio * finalGearRatio : 0.0f;
	}

	void CalculateEngineVelocity(float wheel1AngularVel, float wheel2AngularVel, float clutchInput, float throttleInput)
	{
		float intermediate = FMath::Max((wheel1AngularVel + wheel2AngularVel)/2.0f * totalTransmissionRatio, engineMinSpeed);

		engineAngularVelocity = (totalTransmissionRatio == 0.0f || clutchInput > 0.5)? (engineMaxSpeed - (1-throttleInput)*(engineMaxSpeed-engineMinSpeed)): FMath::Min(intermediate, engineMaxSpeed);
		engineRPM = engineAngularVelocity * 60.0f/(PI * 2.0f);
	}

	void CalculateEngineTorque(float clutchInput, float throttleInput, float carVelocity)
	{
		float intermediate = (clutchInput <= 0.5f)? (1-clutchInput) * throttleInput * (engineMaxTorque - engineMinTorque) + engineMaxTorque : 0.0f;
		currentTorque = (carVelocity < 0.0f)? FMath::Max(0.0f, intermediate) : intermediate;

		drivingTorquePerWheel = (currentTorque * transmissionEfficiency * totalTransmissionRatio)/2.0f * (FMath::Tanh(engineAngularVelocity-(engineMaxSpeed-3.0f))+1)/2.0f;
	}
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
	FWheelLoads CalculateWheelLoads(float bankAngle, float gradientAngle, float accel);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="CarForces")
	TMap<EWheelPosition, FTireInfo> TireMap;
	
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
	
	FVector carVelocity;
	FVector carAcceleration;
	FVector carAngularVelocity;
	FVector carAngularAcceleration;
	
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

	// Tire functions
	void InitialiseTireArray();
	float CalculateLoadChangeFromCorneringForce(float lateralForce, bool forFrontAxel);
	float CalculateWheelLoadRatio();
	void CalculateWheelsLocalVelocities();

	// Car force functions
	float CalculateResistanceForce();
	FCarForces CalculateCarForces();

	// Utility functions
	void ApplyAllAccelerations(float deltaTime);
};
