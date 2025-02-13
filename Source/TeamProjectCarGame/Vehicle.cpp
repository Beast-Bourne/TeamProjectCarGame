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

// Called when the game starts or when spawned
void AVehicle::BeginPlay()
{
	Super::BeginPlay();
	SuspensionMaxLength = WheelRadius + SuspensionLength;
}

// Called every frame
void AVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Suspension simulation
	SuspensionCast(FL_SuspensionMount);
	SuspensionCast(FR_SuspensionMount);
	SuspensionCast(RL_SuspensionMount);
	SuspensionCast(RR_SuspensionMount);
	};

// Function to manage how the car reacts to elevation changes
void AVehicle::SuspensionCast(USceneComponent* Wheel)
{
    float DeltaTime = GetWorld()->GetDeltaSeconds();

    // Line traces to calculate position of the wheel
    FHitResult HitResult;

    // Start trace begins at the Suspension Mount
    FVector StartTrace = Wheel->GetComponentLocation();
    FVector EndTrace = Wheel->GetComponentLocation() + (Wheel->GetUpVector() * -SuspensionMaxLength);
    bool bHit = LineTrace(StartTrace, EndTrace, HitResult, bDebugDraw);
	SuspensionCurrentLength = HitResult.Distance - WheelRadius;

	SuspensionForce = FVector(0, 0, (Stiffness * (RestLength - SuspensionCurrentLength) + (Damping) * (SuspensionPreviousLength - SuspensionCurrentLength) / DeltaTime));
    if (bHit)
    {
		FString VectorString = SuspensionForce.ToString();

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, VectorString);
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

// Called to bind functionality to input
void AVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
}

