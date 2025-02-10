// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicle.h"

#include "Kismet/KismetMathLibrary.h"

// Sets default values
AVehicle::AVehicle()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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
	
}

void AVehicle::SuspensionCast(USceneComponent* Wheel)
{
    float DeltaTime = GetWorld()->GetDeltaSeconds();

	// Line traces to calculate position of the wheel
    FHitResult HitResult;
    FVector StartTrace = Wheel->GetComponentLocation();
    FVector EndTrace = Wheel->GetComponentLocation() + (Wheel->GetUpVector() * -60);
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
    	
    	// Set wheel's location relative to where the Scene component is currently located ( the suspension )
        Wheel->GetChildComponent(0)->SetRelativeLocation(FVector(0, 0, WheelZValue));
    	
    	
    }

	// Simulate gravity if there is no hit being detected
    else
    {
        const float DefaultZ = -25.0f;
        FVector GravityForce = FVector(0, 0, BoxComponent->GetMass() * GetWorld()->GetGravityZ());
    	GravityForce = GravityForce/5;
        BoxComponent->AddForceAtLocation(GravityForce, Wheel->GetComponentLocation());

        // Reset the wheel's height to it's lowest point
        Wheel->GetChildComponent(0)->SetRelativeLocation(FVector(0, 0, DefaultZ));
    }
}



// Called every frame
void AVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SuspensionCast(FL_Wheels);
	SuspensionCast(FR_Wheels);
	SuspensionCast(RL_Wheels);
	SuspensionCast(RR_Wheels);

}

// Called to bind functionality to input
void AVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

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

