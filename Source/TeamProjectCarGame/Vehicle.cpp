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
	FL_SuspensionMount = CreateDefaultSubobject<USceneComponent>(TEXT("FL_Wheels"));
	FR_SuspensionMount = CreateDefaultSubobject<USceneComponent>(TEXT("FR_Wheels"));
	RL_SuspensionMount = CreateDefaultSubobject<USceneComponent>(TEXT("RL_Wheels"));
	RR_SuspensionMount = CreateDefaultSubobject<USceneComponent>(TEXT("RR_Wheels"));

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

	// Attach the Static Meshes to the Scene Components
	FL_WheelMeshes->SetupAttachment(FL_SuspensionMount);
	FR_WheelMeshes->SetupAttachment(FR_SuspensionMount);
	RL_WheelMeshes->SetupAttachment(RL_SuspensionMount);
	RR_WheelMeshes->SetupAttachment(RR_SuspensionMount);

}

void AVehicle::BeginPlay()
{
	Super::BeginPlay();

	// Dynamically calculate the wheel radius for each wheel
	float FL_Radius = GetWheelRadius(FL_WheelMeshes);
	float FR_Radius = GetWheelRadius(FR_WheelMeshes);
	float RL_Radius = GetWheelRadius(RL_WheelMeshes);
	float RR_Radius = GetWheelRadius(RR_WheelMeshes);

	// Average all wheel radii (or handle per-axle adjustments)
	WheelRadius = (FL_Radius + FR_Radius + RL_Radius + RR_Radius) / 4.0f;

	// Set suspension length dynamically (Example: 1.5x Wheel Radius)
	SuspensionLength = WheelRadius * 1.25f;

	// Set the max suspension travel length
	SuspensionMaxLength = WheelRadius + SuspensionLength;

	// Debug Output
	FString DebugMessage = FString::Printf(TEXT("Wheel Radius: %f | Suspension Length: %f"), WheelRadius, SuspensionLength);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, DebugMessage);
}



// Called every frame
void AVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Suspension simulation
	SuspensionCast(FL_SuspensionMount, FL_WheelMeshes);
	SuspensionCast(FR_SuspensionMount, FR_WheelMeshes);
	SuspensionCast(RL_SuspensionMount, RL_WheelMeshes);
	SuspensionCast(RR_SuspensionMount, RR_WheelMeshes);
	};

// Function to manage how the car reacts to elevation changes
void AVehicle::SuspensionCast(USceneComponent* Wheel, UStaticMeshComponent* WheelMesh)
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

	// Calculate suspension force
	SuspensionForce = FVector(0, 0, (Stiffness * (RestLength - SuspensionCurrentLength) +
									(Damping * (SuspensionPreviousLength - SuspensionCurrentLength) / FMath::Max(DeltaTime, KINDA_SMALL_NUMBER))));

	// Apply force if hit
	if (bHit)
	{
		FString DebugMessage = FString::Printf(TEXT("Suspension Force: %s | Length: %f"), *SuspensionForce.ToString(), SuspensionCurrentLength);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, DebugMessage);

		float WheelZValue = HitResult.ImpactNormal.Z - WheelRadius;
		WheelMesh->SetRelativeLocation(FVector(0,0, (WheelZValue)));
		CarBody->AddForceAtLocation(SuspensionForce, HitResult.ImpactPoint);
		SuspensionPreviousLength = SuspensionCurrentLength;
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
	float SweepRadius = WheelRadius * 0.9f;  // Slightly smaller to avoid instant ground contact

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


