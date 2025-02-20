// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicle.h"

#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputComponent.h"

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
	FL_WheelMeshes = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FL_WheelMesh"));
	FR_WheelMeshes = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FR_WheelMesh"));
	RL_WheelMeshes = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RL_WheelMesh"));
	RR_WheelMeshes = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RR_WheelMesh"));

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
	FL_WheelMeshes->SetupAttachment(FL_SuspensionMount);
	FR_WheelMeshes->SetupAttachment(FR_SuspensionMount);
	RL_WheelMeshes->SetupAttachment(RL_SuspensionMount);
	RR_WheelMeshes->SetupAttachment(RR_SuspensionMount);

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

	// Suspension simulation
	SuspensionCast(FL_SuspensionMount, FL_WheelMeshes, FL_SuspensionRest);
	SuspensionCast(FR_SuspensionMount, FR_WheelMeshes, FR_SuspensionRest);
	SuspensionCast(RL_SuspensionMount, RL_WheelMeshes, RL_SuspensionRest);
	SuspensionCast(RR_SuspensionMount, RR_WheelMeshes, RR_SuspensionRest);
	};

// Function to manage how the car reacts to elevation changes
void AVehicle::SuspensionCast(USceneComponent* Wheel, UStaticMeshComponent* WheelMesh, USceneComponent* SuspensionRest)
{
	
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	FHitResult HitResult;
	WheelRadius = GetWheelRadius(WheelMesh);

	// Define the start and end points for the sweep trace
	FVector StartTrace = Wheel->GetComponentLocation();
	FVector EndTrace = StartTrace + (Wheel->GetUpVector() * -SuspensionMaxLength);

	// Use sweep trace instead of line trace
	bool bHit = SweepTrace(StartTrace, EndTrace, HitResult, bDebugDraw);

	// Compute suspension length
	SuspensionCurrentLength = bHit ? (HitResult.Distance - WheelRadius) : SuspensionMaxLength;

	SpringDirection = HitResult.ImpactNormal;

	TireVelocity = CarBody->GetPhysicsLinearVelocityAtPoint(Wheel->GetComponentLocation());

	SuspensionRestDistance = Wheel->GetComponentLocation().Z - SuspensionRest->GetComponentLocation().Z;

	float Offset = SuspensionRestDistance - HitResult.Distance;
	
	float Velocity = FVector::DotProduct(SpringDirection, TireVelocity);

	// Calculate suspension force
	SuspensionForce = (Offset * SuspensionStrength) - (Velocity * Damper);

	// Apply force if hit
	if (bHit)
	{
		CarBody->AddForceAtLocation(SuspensionForce * SpringDirection, HitResult.ImpactPoint);
		SuspensionPreviousLength = SuspensionCurrentLength;

		// Set the wheel mesh to the contact point minus the wheel radius
		FVector NewWheelLocation = HitResult.ImpactPoint + FVector(0,0, WheelRadius);
		WheelMesh->SetWorldLocation(NewWheelLocation);
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

// Called to bind functionality to input
void AVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
}

float AVehicle::GetWheelRadius(UStaticMeshComponent* WheelMesh)
{
	if (!WheelMesh || !WheelMesh->GetStaticMesh()) return 0.0f;  // Safety check

	// Get the bounding box of the mesh
	FVector BoxExtent = WheelMesh->GetStaticMesh()->GetBoundingBox().GetExtent();

	// The radius is half the height (Z-axis extent)
	return BoxExtent.Z;
}


