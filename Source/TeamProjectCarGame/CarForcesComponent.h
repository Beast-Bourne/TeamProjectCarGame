// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Math/UnrealMathUtility.h"
#include "CarForcesComponent.generated.h"

struct FWheelLoads
{
	float FrontRight;
	float FrontLeft;
	float RearRight;
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

USTRUCT()
struct FBrakeInfo
{
	GENERATED_BODY()

	bool isFrontBrake;
	float BrakeDiameter;   // m
	float padArea = 0.007f;             // m^2
	float frictionCoefficient = 0.5f;
	float pressureLimit = 0.7f;         // fraction of the maximum brake pressured for the rear wheels
	float calliperDiameter = 0.036f;    // m
	float maxBrakePressure = 14000.0f;    // kilo-pascals

	FBrakeInfo()
	{
		isFrontBrake = false;
		BrakeDiameter = 0.0f;
	}
	
	FBrakeInfo(bool isFrontBrake)
	{
		this->isFrontBrake = isFrontBrake;
		this->BrakeDiameter = (isFrontBrake)? 0.41f : 0.39f; // .41 for front and .39 for rear
	}

	float CalculateBrakingTorque(float brakeInput, float wheelAngularVel, int gear)
	{
		float pressure = maxBrakePressure * brakeInput;
		float pressureLim = maxBrakePressure * pressureLimit;
		float brakePressure = (!isFrontBrake && pressure >= pressureLim)? pressureLim : pressure;

		float part1 = 2.0f * (0.58f*BrakeDiameter) * brakePressure * 10.0f;
		float part2 = (PI/4.0f * calliperDiameter*calliperDiameter)/padArea;
		float part3 = PI * frictionCoefficient * (BrakeDiameter*BrakeDiameter - FMath::Square(0.58f *BrakeDiameter));

		return -part1 * part2 * part3 * FMath::Tanh(wheelAngularVel);
	}
};

USTRUCT(BlueprintType)
struct FTireInfo
{
	GENERATED_BODY()
	
	FVector localVelocity = FVector::ZeroVector;             // velocity of the tire relative to the ground
	UPROPERTY(BlueprintReadOnly)
	float angularVelocity = 0.0f;                            // angular velocity of the tire
	float slipVelocity = 0.0f;                               // velocity of the tire in a no-slip condition
	float longitudinalSlipFactor = 0.0f;                     // slip of the tire in the longitudinal direction
	float lateralSlipFactor = 0.0f;                          // slip of the tire in the lateral direction
	float combinedSlipFactor= 0.0f;           // slip factor used for determining conditions for calculating slip forces
	float slipConditionFactor = 0.0f;         // factor used for checking if the tire is working in linear or sliding conditions
	float threadStiffness = 0.0f;
	float staticFrictionCoefficient = 1.2f;
	float slidingFrictionCoefficient = 0.5f;
	float xSlipFriction = 0.0f;
	float ySlipFriction = 0.0f;
	float patchLength = 0.0f;
	float selfAligningTorque = 0.0f;
	float casterOffset = 0.0f;
	float relaxationConstant = 0.0f;

	float xSlipForceWithRelaxation = 0.0f;
	float ySlipForceWithRelaxation = 0.0f;
	float selfAligningTorqueWithRelaxation = 0.0f;
	
	UPROPERTY(BlueprintReadOnly)
	float load = 0.0f; // normal force acting on the tire in kg
	UPROPERTY(BlueprintReadOnly)
	float localLongitudinalForce = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	float localLateralForce = 0.0f;
	float localSelfAligningTorque = 0.0f;
	UPROPERTY(BlueprintReadWrite)
	float delta = 0.0f; // The angle between the tires local forward direction and the cars forward direction
	float offsetFromCentreOfMass = 0.0f; // offset (in x-axis) between the wheel and the car's CoM
	float trackWidth = 0.0f; // width of the axel
	float radius = 0.0f; // tire's radius

	float momentOfInertia = 2.2f; // hard coded (CHANGE LATER)
	float angularAcceleration = 0.0f;

	FBrakeInfo brake;

	// default constructor
	FTireInfo()
	{
		
	}
	
	// main constructor
	FTireInfo(float load, float offset, float trackWidth, float radius, float threadStiffness, float patchLength, float casterOffset, float relaxationConstant, bool isFront)
	{
		this->load = load;
		this->offsetFromCentreOfMass = offset;
		this->trackWidth = trackWidth;
		this->radius = radius;
		this->threadStiffness = threadStiffness;
		this->patchLength = patchLength;
		this->casterOffset = casterOffset;
		this->relaxationConstant = relaxationConstant;
		brake = FBrakeInfo(isFront);
	}

	// sets the local velocity of this tire to the cars velocity and corrects it for any angular velocity about the yaw of the car
	void CalculateLocalVelocity(float xMultiplier, float yMultiplier, const FVector &carVelocity, float carYawAngularVelocity)
	{
		float xValue = carVelocity.X + (xMultiplier * carYawAngularVelocity * trackWidth * 0.5f);
		float yValue = offsetFromCentreOfMass * carYawAngularVelocity + yMultiplier * carVelocity.Y;
		float zValue = localVelocity.Z;
		localVelocity = FVector(xValue, yValue, zValue);
	}

	void UpdateForces(float resistForce, float drivingForce)
	{
		localLongitudinalForce = resistForce + drivingForce;
		localLateralForce = ySlipForceWithRelaxation;
		localSelfAligningTorque = selfAligningTorqueWithRelaxation;

		localLongitudinalForce = FMath::Clamp(localLongitudinalForce, -staticFrictionCoefficient*load, staticFrictionCoefficient*load);
	}
};

struct FCarForces
{
	float longitudinalForce = 0.0f;
	float lateralForce = 0.0f;
	float angularForce = 0.0f;
};

USTRUCT(BlueprintType)
struct FEngineInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float engineAngularVelocity = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	float engineRPM = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float engineMaxTorque = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	float engineMinTorque = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float currentTorque = 0.0f;
	
	float gearTransmissionRatio = 0.0f; // the ratio is 0 for neutral.
	UPROPERTY(BlueprintReadOnly)
	int currentGear = 0;
	float finalGearRatio = 3.86f; // Hard coded for car being used (CHANGE LATER)
	float  totalTransmissionRatio = 0.0f;
	float engineMaxSpeed = 943.0f; // Hard coded for car being used (CHANGE LATER)
	float engineMinSpeed = 89.0f; // Hard coded for car being used (CHANGE LATER)

	float transmissionEfficiency = 0.97f;

	UPROPERTY(BlueprintReadOnly)
	float drivingTorquePerWheel = 0.0f;

	float gearR = 0.0f;
	float gear1 = 0.0f;
	float gear2 = 0.0f;
	float gear3 = 0.0f;
	float gear4 = 0.0f;
	float gear5 = 0.0f;
	float gear6 = 0.0f;

	// initialises variables based on the car being stationary, turned on and in gear 1
	FEngineInfo()
	{
		SwapGears(0);
		CalculateEngineVelocity(0,0,0,0);
		CalculateEngineTorqueRange();
	}

	FEngineInfo(float gearR, float gear1, float gear2, float gear3, float gear4, float gear5, float gear6, float engineMinSpeed, float engineMaxSpeed)
	{
		this->gearR = gearR;
		this->gear1 = gear1;
		this->gear2 = gear2;
		this->gear3 = gear3;
		this->gear4 = gear4;
		this->gear5 = gear5;
		this->gear6 = gear6;
		this->engineMinSpeed = engineMinSpeed;
		this->engineMaxSpeed = engineMaxSpeed;

		SwapGears(0);
		CalculateEngineVelocity(0,0,0,0);
		CalculateEngineTorqueRange();
	}

	void CalculateEngineTorqueRange()
	{
		engineMaxTorque = (2.1486f * FMath::Pow(10.0f, -6) * FMath::Pow(engineAngularVelocity, 3))
		- (3.7390514f * FMath::Pow(10.0f, -6.0f) * engineAngularVelocity * engineAngularVelocity)
		+ (1.8250297732f * engineAngularVelocity);

		engineMaxTorque = (1.7714f * FMath::Pow(10.0f, -7)* FMath::Pow(engineAngularVelocity, 3))
		- 0.00128f * engineAngularVelocity * engineAngularVelocity
		+ 1.463f * engineAngularVelocity
		+10.76f;

		engineMinTorque = (2.152813f * FMath::Pow(10.0f, -4) * engineAngularVelocity * engineAngularVelocity)
		- (0.2413794863f * engineAngularVelocity);
	}

	// hard coded gear ratio values (CHANGE THIS LATER)
	void SwapGears(int newGear)
	{
		currentGear = newGear;
		gearTransmissionRatio = (currentGear == -1)? gearR:
		(currentGear == 0)? 0.0f :
		(currentGear == 1)? gear1:
		(currentGear == 2)? gear2:
		(currentGear == 3)? gear3:
		(currentGear == 4)? gear4:
		(currentGear == 5)? gear5:
		gear6;

		CalculateTotalGearRatio();
	}

	void CalculateTotalGearRatio()
	{
		totalTransmissionRatio = (currentGear != 0) ? gearTransmissionRatio * finalGearRatio : 0.0f;
	}

	// calculates the engine's angular velocity (and RPM) based on the clutch/throttle inputs
	void CalculateEngineVelocity(float wheel1AngularVel, float wheel2AngularVel, float clutchInput, float throttleInput)
	{
		float averageWheelSpeed = FMath::Abs((wheel1AngularVel + wheel2AngularVel)/2.0f);
		float intermediate = FMath::Max( averageWheelSpeed * totalTransmissionRatio, engineMinSpeed);

		engineAngularVelocity = (totalTransmissionRatio == 0.0f || clutchInput > 0.5)? (engineMaxSpeed - (1-throttleInput)*(engineMaxSpeed-engineMinSpeed)): FMath::Min(intermediate, engineMaxSpeed);
		engineRPM = engineAngularVelocity * 60.0f/(PI * 2.0f);
	}

	// calculates the torque produced by the engine and the torque applied to each wheel
	// the intermediate stores the calculation of the torque value
	// the currentTorque is then set based on some conditions. No torque is provided to the wheels when the clutch is pressed (>0.5) or when the cars in neutral
	void CalculateEngineTorque(float clutchInput, float throttleInput, float carVelocity)
	{
		float intermediate = (clutchInput <= 0.5f)? (1-clutchInput) * throttleInput * (engineMaxTorque - engineMinTorque) + engineMinTorque : 0.0f;
		currentTorque = (carVelocity < 0.0f)? FMath::Max(0.0f, intermediate) : intermediate;

		drivingTorquePerWheel = (currentTorque * transmissionEfficiency * totalTransmissionRatio)/2.0f * -(FMath::Tanh(engineAngularVelocity-(engineMaxSpeed-3.0f))-1)/2.0f;
		drivingTorquePerWheel *= (currentGear == -1)? -1.0f : 1.0f;
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

	void CalculateWheelLoads(float bankAngle, float gradientAngle, float accel);

	UPROPERTY(BlueprintReadOnly, category="CarForces")
	FTireInfo tireFR;
	UPROPERTY(BlueprintReadOnly, category="CarForces")
	FTireInfo tireFL;
	UPROPERTY(BlueprintReadOnly, category="CarForces")
	FTireInfo tireRR;
	UPROPERTY(BlueprintReadOnly, category="CarForces")
	FTireInfo tireRL;

	UPROPERTY(BlueprintReadOnly)
	FEngineInfo engineInfo;

	UPROPERTY(BlueprintReadOnly)
	float totalWheelLoadToCarWeightRatio;

	UPROPERTY(BlueprintReadWrite)
	float longitudinalAcceleration;

	UPROPERTY(BlueprintReadOnly)
	float lateralAcceleration;

	UPROPERTY(BlueprintReadWrite)
	float clutchInput;

	UPROPERTY(BlueprintReadWrite)
	float throttleInput;

	UPROPERTY(BlueprintReadWrite)
	float brakeInput;

	UPROPERTY(BlueprintReadOnly)
	FVector carVelocity;

	UPROPERTY(BlueprintReadOnly)
	FVector carAcceleration;

	UPROPERTY(BlueprintReadOnly)
	FVector carAngularVelocity;

	// blueprintable functions
	UFUNCTION(BlueprintCallable)
	void PerformSimulationFrame(float deltaTime, bool carIsGrounded, bool carIsWallDragging, bool frontWallCollision, bool backWallCollision);
	UFUNCTION(BlueprintCallable)
	void ResetCarSpeed();

	// Main body constants
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|General")
	float mass = 1450.0f; // mass in Kg
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|General")
	float centreOfMassHeight = 0.48f; // height of the centre of mass above the ground in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|Wheels")
	float wheelSeparation = 2.457f; // horizontal distance between the front and rear axles in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|Wheels")
	float frontWheelOffset = 0.957f; // horizontal offset from the centre of gravity in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|Wheels")
	float rearWheelOffset = 1.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|Wheels")
	float frontTrackWidth = 1.6f; // the width of the front axle in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|Wheels")
	float rearTrackWidth = 1.56f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|General")
	float rollCentreHeight = 0.1f; // height of the roll centre above the ground in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|Wheels")
	float frontSuspensionStiffness = 120000.0f; // suspension in N/m
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|Wheels")
	float rearSuspensionStiffness = 200000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|General")
	float frontArea = 2.2f; // frontal area in m^2
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|General")
	float dragCoefficient = 0.39f; // the drag coefficient used for calculating the air resistance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|Wheels")
	float rollResistanceCoefficient = 0.012f; // the rolling resistance coefficient used for calculating the roll resistance force
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|Gears")
	float reverseGearRatio = 3.42f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|Gears")
	float FirstGearRatio = 3.92f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|Gears")
	float SecondGearRatio = 2.29f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|Gears")
	float ThirdGearRatio = 1.55f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|Gears")
	float ForthGearRatio = 1.18f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|Gears")
	float FifthGearRatio = 0.94f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|Gears")
	float SixthGearRatio = 0.79f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|Gears")
	float minEngineSpeed = 89.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category= "Variables|Gears")
	float maxEngineSpeed = 943.0f;
	
private:
	// non constants
	float carSpeed = 0.0f; // vehicles forward velocity in mph
	float acceleration = 0.0f; // vehicles forward acceleration in m/s^2
	float gradient = 0.0f; // vehicles pitch in radians (positive when the front is above the rear)
	float bank = 0.0f; // vehicles roll in radians (positive when the left is above the right)
	
	FVector carAngularAcceleration;
	
	// Main body constants
	const float momentOfInertiaX = 2000.0f; // moment of inertia about the X axis in kg m^2
	const float momentOfInertiaY = 3000.0f;
	const float momentOfInertiaZ = 5000.0f;
	const float airDensity = 1.225f; // density of the air in kg/m^3
	const float g = 9.81f; // gravity

	// Tire constants
	const FWheelLoads staticWheelLoads = CalculateStaticWheelLoads();
	const float tireRadius = 0.350f; // tire radius in meters
	const float tireInertia = 1.0f; // the moment of inertia of the tires in kg m^2
	const float relaxationLengthCoefficient = 0.5f;
	const float threadStiffness = 30.0f; // N/m
	const float casterOffset = 0.03f; // m
	const float contactPatchLength = 0.1f; // m
	const float slidingFrictionConstant = 0.7f; // for dry asphalt

	// Tire functions
	float CalculateLoadChangeFromCorneringForce(float lateralForce, bool forFrontAxel);
	float CalculateWheelLoadRatio();
	void CalculateWheelsLocalVelocities();
	FWheelLoads CalculateStaticWheelLoads();

	// Car force functions
	void CalculateWheelForces(bool carIsGrounded, bool carIsWallDragging, bool frontWallCollision, bool backWallCollision);
	FCarForces CalculateCarForces();

	// Utility functions
	void ApplyAllAccelerations(float deltaTime);
	void CalculateCarAngularVelocity();
	void CheckForGearShift();
	void CheckForAdditionalBraking();
};
