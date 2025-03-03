// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicle.h"

#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AVehicle::AVehicle()
{
	PrimaryActorTick.bCanEverTick = true;

	CarBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Car Body"));

	// Wheel Scene Components for suspension calculations
	FL_SuspensionMount = CreateDefaultSubobject<USceneComponent>(TEXT("FL_Suspension"));
	FR_SuspensionMount = CreateDefaultSubobject<USceneComponent>(TEXT("FR_Suspension"));
	RL_SuspensionMount = CreateDefaultSubobject<USceneComponent>(TEXT("RL_Suspension"));
	RR_SuspensionMount = CreateDefaultSubobject<USceneComponent>(TEXT("RR_Suspension"));
	
	FL_SuspensionRest = CreateDefaultSubobject<USceneComponent>(TEXT("FL_SuspensionRest"));
	FR_SuspensionRest = CreateDefaultSubobject<USceneComponent>(TEXT("FR_SuspensionRest"));
	RL_SuspensionRest = CreateDefaultSubobject<USceneComponent>(TEXT("RL_SuspensionRest"));
	RR_SuspensionRest = CreateDefaultSubobject<USceneComponent>(TEXT("RR_SuspensionRest"));

	// Meshes used to dynamically display wheel's current location
	FL_TireMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FL_TireMesh"));
	FR_TireMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FR_TireMesh"));
	RL_TireMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RL_TireMesh"));
	RR_TireMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RR_TireMesh"));

	FL_RimMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FL_RimMesh"));
	FR_RimMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FR_RimMesh"));
	RL_RimMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RL_RimMesh"));
	RR_RimMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RR_RimMesh"));

	// Attach all to the Root Component
	RootComponent = CarBody;
	FL_SuspensionMount->SetupAttachment(RootComponent);
	FR_SuspensionMount->SetupAttachment(RootComponent);
	RL_SuspensionMount->SetupAttachment(RootComponent);
	RR_SuspensionMount->SetupAttachment(RootComponent);

	FL_SuspensionRest->SetupAttachment(FL_SuspensionMount);
	FR_SuspensionRest->SetupAttachment(FR_SuspensionMount);
	RL_SuspensionRest->SetupAttachment(RL_SuspensionMount);
	RR_SuspensionRest->SetupAttachment(RR_SuspensionMount);
	
	// Attach the Static Meshes to the Scene Components
	FL_TireMesh->SetupAttachment(FL_SuspensionMount);
	FR_TireMesh->SetupAttachment(FR_SuspensionMount);
	RL_TireMesh->SetupAttachment(RL_SuspensionMount);
	RR_TireMesh->SetupAttachment(RR_SuspensionMount);

	FL_RimMesh->SetupAttachment(FL_TireMesh);
	FR_RimMesh->SetupAttachment(FR_TireMesh);
	RL_RimMesh->SetupAttachment(RL_TireMesh);
	RR_RimMesh->SetupAttachment(RR_TireMesh);

}

void AVehicle::BeginPlay()
{
	Super::BeginPlay();

	// Set the max suspension travel length
	SuspensionMaxLength = WheelRadius + SuspensionRestDistance;
}



// Called every frame
void AVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);



	};

// Function to manage how the car reacts to elevation changes
void AVehicle::SuspensionCast(USceneComponent* Wheel, UStaticMeshComponent* WheelMesh, USceneComponent* SuspensionRest, float SuspensionStrength, float WheelLoad, bool DebugDraw)
{
	
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	FHitResult HitResult;
	WheelRadius = GetWheelRadius(WheelMesh);

	// Define the start and end points for the sweep trace
	FVector StartTrace = Wheel->GetComponentLocation();
	FVector EndTrace = StartTrace + (Wheel->GetUpVector() * -SuspensionMaxLength);

	// Use sweep trace instead of line trace
	bool bHit = SweepTrace(StartTrace, EndTrace, HitResult, DebugDraw);

	// Compute suspension length
	SuspensionCurrentLength = bHit ? (HitResult.Distance - WheelRadius) : SuspensionMaxLength;

	FVector SpringDirection = HitResult.ImpactNormal;

	TireVelocity = CarBody->GetPhysicsLinearVelocityAtPoint(Wheel->GetComponentLocation());

	SuspensionRestDistance = Wheel->GetComponentLocation().Z - SuspensionRest->GetComponentLocation().Z;

	float Offset = SuspensionRestDistance - HitResult.Distance;

	float ClampedOffset = FMath::Clamp(Offset, -5, 10);
	
	float Velocity = FVector::DotProduct(SpringDirection, TireVelocity);

	// Calculate suspension force
	SuspensionForce = (Offset * SuspensionStrength * WheelLoad) - (Velocity * Damper);

	// Apply force if hit
	if (bHit)
	{
		CarBody->AddForceAtLocation(SuspensionForce * SpringDirection, HitResult.ImpactPoint);
		SuspensionPreviousLength = SuspensionCurrentLength;

		FVector NewWheelLocation = HitResult.Location;
		WheelMesh->SetWorldLocation(NewWheelLocation);
	}
	else
	{
		SuspensionForce = 0;
		
	}

	if (Wheel->GetName() == "FR_Suspension")
	{
		FR_SuspensionForce = SuspensionForce;
		FR_SuspensionOffset = ClampedOffset;
		FR_SuspensionDirection = SpringDirection;
		FR_SpringVelocity = Velocity;
	}
	if (Wheel->GetName() == "FL_Suspension")
	{
		FL_SuspensionForce = SuspensionForce;
		FL_SuspensionOffset = ClampedOffset;
		FL_SuspensionDirection = SpringDirection;
		FL_SpringVelocity = Velocity;
	}
	if (Wheel->GetName() == "RR_Suspension")
	{
		RR_SuspensionForce = SuspensionForce;
		RR_SuspensionOffset = ClampedOffset;
		RR_SuspensionDirection = SpringDirection;
		RR_SpringVelocity = Velocity;
	}
	if (Wheel->GetName() == "RL_Suspension")
	{
		RL_SuspensionForce = SuspensionForce;
		RL_SuspensionOffset = ClampedOffset;
		RL_SuspensionDirection = SpringDirection;
		RL_SpringVelocity = Velocity;
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

bool AVehicle::SweepTrace(FVector StartLocation, FVector EndLocation, FHitResult& OutHitResult, bool bDrawDebug) const
{
	// Define a sphere radius (should be smaller than or equal to the wheel radius)
	float SweepRadius = WheelRadius;

	// Collision query parameters
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	// Define the shape of the sweep (sphere)
	FCollisionShape Sphere = FCollisionShape::MakeSphere(SweepRadius);

	// Perform the sweep trace
	bool bHit = GetWorld()->SweepSingleByChannel(
		OutHitResult, 
		StartLocation, 
		EndLocation, 
		FQuat::Identity,  // No rotation for a sphere
		ECC_Visibility, 
		Sphere, 
		CollisionParams
	);

	// Debug visualization
	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), OutHitResult.Location, SweepRadius, 12, bHit ? FColor::Green : FColor::Red, false, 1.0f);
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Blue, false, 1.0f, 0, 1.0f);
	}

	return bHit;
}

void AVehicle::RunSimulationFrame(float FR_WheelLoad, float FL_WheelLoad, float RR_WheelLoad, float RL_WheelLoad, FVector ResultantForce, FVector Velocity)
{
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// Suspension simulation
	SuspensionCast(FL_SuspensionMount, FL_TireMesh, FL_SuspensionRest, FrontSuspensionStrength, FL_WheelLoad, false);
	SuspensionCast(FR_SuspensionMount, FR_TireMesh, FR_SuspensionRest, FrontSuspensionStrength, FR_WheelLoad, false);
	SuspensionCast(RL_SuspensionMount, RL_TireMesh, RL_SuspensionRest, RearSuspensionStrength, RL_WheelLoad, true);
	SuspensionCast(RR_SuspensionMount, RR_TireMesh, RR_SuspensionRest, RearSuspensionStrength, RR_WheelLoad, true);

	ApplyAccelerationForce(RL_SuspensionMount, RL_TireMesh, ResultantForce, Velocity);
	ApplyAccelerationForce(RR_SuspensionMount, RR_TireMesh, ResultantForce, Velocity);

	if (GEngine)
	{
		FString Message1 = FString::Printf(TEXT("Wheel Radius: %f"), WheelRadius);
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, Message1);

		FString Message2 = FString::Printf(TEXT("FR_SuspensionForce: %f, FL_SuspensionForce: %f"), FR_SuspensionForce, FL_SuspensionForce);
		GEngine->AddOnScreenDebugMessage(2, 5.f, FColor::Red, Message2);
		FString Message3 = FString::Printf(TEXT("RR_SuspensionForce: %f, RL_SuspensionForce: %f,"), RR_SuspensionForce, RL_SuspensionForce);
		GEngine->AddOnScreenDebugMessage(3, 5.f, FColor::Red, Message3);

		FString Message4 = FString::Printf(TEXT("FR_SuspensionOffset: %f, FL_SuspensionOffset: %f"), FR_SuspensionOffset, FL_SuspensionOffset);
		GEngine->AddOnScreenDebugMessage(5, 5.f, FColor::Red, Message4);
		FString Message5 = FString::Printf(TEXT("RR_SuspensionOffset: %f, RL_SuspensionOffset: %f"), RR_SuspensionOffset, RL_SuspensionOffset);
		GEngine->AddOnScreenDebugMessage(6, 5.f, FColor::Red, Message5);

		FString Message8 = FString::Printf(TEXT("FR_SpringVelocity %f, FL_SpringVelocity: %f"), FR_SpringVelocity, FL_SpringVelocity);
		GEngine->AddOnScreenDebugMessage(2, 5.f, FColor::Red, Message8);
		FString Message9 = FString::Printf(TEXT("RR_SpringVelocity: %f, RL_SpringVelocity: %f,"), RR_SpringVelocity, RL_SpringVelocity);
		GEngine->AddOnScreenDebugMessage(3, 5.f, FColor::Red, Message9);

		FString Message6 = FString::Printf(TEXT("FR_SuspensionDirection: X = %f, Y = %f, Z = %f, FL_SuspensionDirection: X = %f, Y = %f, Z = %f,"), FR_SuspensionDirection.X, FR_SuspensionDirection.Y, FR_SuspensionDirection.Z, FL_SuspensionDirection.X, FL_SuspensionDirection.Y, FL_SuspensionDirection.Z);
		GEngine->AddOnScreenDebugMessage(8, 5.f, FColor::Red, Message6);
		FString Message7 = FString::Printf(TEXT("RR_SuspensionDirection: X = %f, Y = %f, Z = %f, RL_SuspensionDirection: X = %f, Y = %f, Z = %f,"), RR_SuspensionDirection.X, RR_SuspensionDirection.Y, RR_SuspensionDirection.Z, RL_SuspensionDirection.X, RL_SuspensionDirection.Y, RL_SuspensionDirection.Z);
		GEngine->AddOnScreenDebugMessage(9, 5.f, FColor::Red, Message7);
	}
}

void AVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
}


float AVehicle::GetWheelRadius(UStaticMeshComponent* WheelMesh)
{
	if (!WheelMesh || !WheelMesh->GetStaticMesh()) return 0.0f;  // Safety check

	// Get the bounding box of the mesh
	FVector BoxExtent = WheelMesh->GetStaticMesh()->GetBoundingBox().GetExtent();

	// The radius is half the height (Z-axis extent)
	return BoxExtent.Z;
}

void AVehicle::ApplyAccelerationForce(USceneComponent* Wheel, UStaticMeshComponent* WheelMesh, FVector ResultantForce, FVector Velocity)
{
	// Get the force application point
	FVector ForceApplicationPoint = WheelMesh->GetComponentLocation();
	ForceApplicationPoint.Z = GetActorLocation().Z;
	FVector ForceMagnitude = CarBody->GetForwardVector() * (Velocity.X * 145000);
	FString Message10 = FString::Printf(TEXT("RR_SuspensionOffset: %f"), ForceMagnitude.X);
	GEngine->AddOnScreenDebugMessage(10, 5.f, FColor::Red, Message10);
	CarBody->AddForce(ForceMagnitude);

}
