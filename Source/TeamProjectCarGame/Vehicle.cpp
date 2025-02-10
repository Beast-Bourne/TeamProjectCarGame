// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicle.h"

#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputComponent.h"

// Sets default values
AVehicle::AVehicle()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create and attach main collision body
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));

	// Wheel Scene Components for suspension calculations
	FL_Wheels = CreateDefaultSubobject<USceneComponent>(TEXT("FL_Wheels"));
	FR_Wheels = CreateDefaultSubobject<USceneComponent>(TEXT("FR_Wheels"));
	RL_Wheels = CreateDefaultSubobject<USceneComponent>(TEXT("RL_Wheels"));
	RR_Wheels = CreateDefaultSubobject<USceneComponent>(TEXT("RR_Wheels"));

	// Meshes used to dynamically display wheel's current location
	FL_WheelMeshes = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FL_WheelMesh"));
	FR_WheelMeshes = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FR_WheelMesh"));
	RL_WheelMeshes = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RL_WheelMesh"));
	RR_WheelMeshes = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RR_WheelMesh"));

	// Attach all to the Root Component
	RootComponent = BoxComponent;
	FL_Wheels->SetupAttachment(RootComponent);
	FR_Wheels->SetupAttachment(RootComponent);
	RL_Wheels->SetupAttachment(RootComponent);
	RR_Wheels->SetupAttachment(RootComponent);

	// Attach the Static Meshes to the Scene Components
	FL_WheelMeshes->SetupAttachment(FL_Wheels);
	FR_WheelMeshes->SetupAttachment(FR_Wheels);
	RL_WheelMeshes->SetupAttachment(RL_Wheels);
	RR_WheelMeshes->SetupAttachment(RR_Wheels);

}

// Called when the game starts or when spawned
void AVehicle::BeginPlay()
{
	Super::BeginPlay();

	// Store the default acceleration values
	DefaultAccelerationMultiplier = AccelerationMultiplier;
}

// Called every frame
void AVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Suspension simulation
	SuspensionCast(FL_Wheels);
	SuspensionCast(FR_Wheels);
	SuspensionCast(RL_Wheels);
	SuspensionCast(RR_Wheels);

	GroundCheck();
	};

// Function to calculate the Acceleration Values based on player input
void AVehicle::Accelerate(const FInputActionInstance& ActionValue)
{
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	if (bCanAccelerate)
	{
		float InputValue = ActionValue.GetValue().Get<float>();
		
		// Make sure vehicle is on the ground before interpolating the Input
		float TargetValue = bGroundCheckResult ? InputValue : 0.0f;
		AccelerationInput = FMath::FInterpTo(AccelerationInput, TargetValue, DeltaTime, AccelerationInterpolationSpeed);

		// Rotate wheels towards their original position if turning
		if (FL_Wheels && FR_Wheels)
        {
			float CurrentWheelYaw = FR_Wheels->GetRelativeRotation().Yaw;
			float NewWheelYaw = FMath::FInterpTo(CurrentWheelYaw, 0.0f, DeltaTime, WheelInterpolationSpeed);
            FL_Wheels->SetRelativeRotation(FRotator(0,NewWheelYaw,0));
            FR_Wheels->SetRelativeRotation(FRotator(0,NewWheelYaw,0));
			
        }

	}
}

// Function to handle how the vehicle steers
void AVehicle::Steer(const FInputActionInstance& ActionValue)
{
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	SteeringInputValue = ActionValue.GetValue().Get<float>();
	bIsTurning = true;
	
	float CurrentWheelYaw = FL_Wheels->GetRelativeRotation().Yaw;

	ZTorque = BaseTurningTorque;

	// Set wheel rotation based on the current torque being applied to the wheels
	if (ZTorque < MaxZTorque && !bAirControl)
	{
		float SteeringAngleYaw = FMath::FInterpTo(CurrentWheelYaw, (SteeringInputValue * MaxSteeringAngle), DeltaTime, WheelInterpolationSpeed);
		FL_Wheels->SetRelativeRotation(FRotator(0, SteeringAngleYaw, 0));
		FR_Wheels->SetRelativeRotation(FRotator(0, SteeringAngleYaw, 0));
		BoxComponent->AddTorqueInRadians(FVector(0, 0, (SteeringInputValue * ZTorque * AccelerationInput)));
	}
	else if (!bAirControl)
	{
		float SteeringAngleYaw = FMath::FInterpTo(CurrentWheelYaw, (SteeringInputValue * MinSteeringAngle), DeltaTime, WheelInterpolationSpeed);
		FL_Wheels->SetRelativeRotation(FRotator(0, SteeringAngleYaw, 0));
		FR_Wheels->SetRelativeRotation(FRotator(0, SteeringAngleYaw, 0));
		BoxComponent->AddTorqueInRadians(FVector(0, 0, (SteeringInputValue * ZTorque * AccelerationInput)));
	}
	else
	{
		BoxComponent->AddTorqueInRadians(FVector(0, 0, SteeringInputValue * ZTorque * AccelerationInput));
	}
}

// Function to make sure the player isn't able to accelerate or control the car when they're in the air
void AVehicle::GroundCheck()
{
	FHitResult HitResult;
	
	// Line trace to detect whether there is a hit
    bool bHit = LineTrace(GetActorLocation(),GetActorLocation()+FVector(0,0,-100),HitResult, bDebugDraw);
    if (bHit)
       {
    	bAirControl = false;
    	bCanAccelerate = true;
       }
    else
       {
    	bCanAccelerate = false;
    	bAirControl = false;
       }
 
}

// Function to set Acceleration values accordingly to the vehicle's properties
void AVehicle::CalculateAcceleration()
{
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	Acceleration = FMath::Lerp(MinAccelerationLerp, MaxAccelerationLerp, AccelerationInput);
	AccelerationInput = FMath::FInterpTo(AccelerationInput, 0.0, DeltaTime, DecelerationInterpolationSpeed);
	float RotationValue = (Acceleration * AccelerationInput) * -AccelerationInput / 20;


	// Simulate wheel rotation
	FL_WheelMeshes->AddLocalRotation(FRotator(0,0,RotationValue));
	FR_WheelMeshes->AddLocalRotation(FRotator(0,0,RotationValue));
	RL_WheelMeshes->AddLocalRotation(FRotator(0,0,RotationValue));
	RR_WheelMeshes->AddLocalRotation(FRotator(0,0,RotationValue));
}

// Function to apply acceleration to the vehicle once it has been calculated and there is a ground hit result
void AVehicle::AccelerateVehicle(USceneComponent* Wheel)
{
	FVector Force = BoxComponent->GetForwardVector() * BoxComponent->GetMass() * AccelerationMultiplier * AccelerationInput;

	// Apply the force closer to the ground to reduce tilt
	FVector ForceApplicationPoint = Wheel->GetComponentLocation();
	ForceApplicationPoint.Z = GetActorLocation().Z;
	
	BoxComponent->AddForceAtLocation(Force, ForceApplicationPoint);
}

// Function to manage how the car reacts to elevation changes
void AVehicle::SuspensionCast(USceneComponent* Wheel)
{
    float DeltaTime = GetWorld()->GetDeltaSeconds();

	// Line traces to calculate position of the wheel
    FHitResult HitResult;
    FVector StartTrace = Wheel->GetComponentLocation();
    FVector EndTrace = Wheel->GetComponentLocation() + (Wheel->GetUpVector() * -100);
    bool bHit = LineTrace(StartTrace, EndTrace, HitResult, bDebugDraw);

    if (bHit)
    {
        float NormalizedValue = UKismetMathLibrary::NormalizeToRange(HitResult.Distance, 0, 50);
        NormalizedValue = 1.2f - NormalizedValue;
        bIsSuspensionActive = true;

        // Calculate the direction of the force for suspension
        FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(HitResult.TraceEnd, HitResult.TraceStart);
        FVector Force = SuspensionForce * NormalizedValue * Direction;
        FVector Location = Wheel->GetComponentLocation();
    	
        BoxComponent->AddForceAtLocation(Force, Location);

    	// Set initial wheel height
        float WheelZLocation = -HitResult.ImpactPoint.Z;
        float InterpTarget = HitResult.Distance * -1.0f;
    	
        float WheelZValue = FMath::FInterpTo(WheelZLocation, InterpTarget, DeltaTime, SuspensionCastInterpolationSpeed);
    	
        const float MinZLocation = -25.0f; // Minimum downward extension
        const float MaxZLocation = -20.0f;  // Maximum upward compression

    	// Clamp height of the wheel between the two values to reduce any instances of wheels going below the ground or above the car's body
        WheelZValue = FMath::Clamp(WheelZValue, MinZLocation, MaxZLocation);

    	// Apply acceleration accordingly
    	AccelerateVehicle(Wheel);

    	// Set wheel's location relative to where the Scene component is currently located ( the suspension )
        Wheel->GetChildComponent(0)->SetRelativeLocation(FVector(0, 0, WheelZValue));
    	
    	
    }

	// Simulate gravity if there is no hit being detected
    else
    {
        const float DefaultZ = -25.0f;
        FVector GravityForce = FVector(0, 0, BoxComponent->GetMass() * GetWorld()->GetGravityZ());
        BoxComponent->AddForceAtLocation(GravityForce, Wheel->GetComponentLocation());
    	
    	AccelerateVehicle(Wheel);

        // Reset the wheel's height to it's lowest point
        Wheel->GetChildComponent(0)->SetRelativeLocation(FVector(0, 0, DefaultZ));
    }
}

// Function which handles how the debug options are being drawn/displayed
void AVehicle::Debug()
{
	bDebugDraw = !bDebugDraw;
	if (GEngine)
	{
		FString DebugState = bDebugDraw ? TEXT("Debug ON") : TEXT("Debug OFF");
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, DebugState);
	}
}


// Function to create a line trace
bool AVehicle::LineTrace(FVector StartLocation, FVector EndLocation, FHitResult& OutHitResult, bool bDrawDebug) const
{
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		OutHitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility, 
		CollisionParams
	);

	// Check whether to draw the line traces depending on debug mode state
	if (bDrawDebug)
	{
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, bHit ? FColor::Green : FColor::Red, false, 1.0f, 0, 1.0f);
	}

	return bHit;
	
}

// Function used for tyre wear calculations and passing in whether steering is no longer applied
void AVehicle::StopSteering()
{
	SteeringInputValue = 0;
	bIsTurning = false;
}

// Called to bind functionality to input
void AVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	Input->BindAction(AccelerateInputAction, ETriggerEvent::Triggered, this, &AVehicle::Accelerate);
	Input->BindAction(SteerInputAction, ETriggerEvent::Triggered, this, &AVehicle::Steer);
	Input->BindAction(SteerInputAction, ETriggerEvent::Completed, this, &AVehicle::StopSteering);
}

