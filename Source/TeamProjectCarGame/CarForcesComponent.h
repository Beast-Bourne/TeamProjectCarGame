// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
	FWheelLoads CalculateSlopedWheelLoads(float bankAngle, float gradientAngle, float accel);

	UPROPERTY(BlueprintReadOnly)
	FWheelLoads wheelLoads; // loads on each tire in kg

	UPROPERTY(BlueprintReadOnly)
	float totalWheelLoadToCarWeightRatio;

	UPROPERTY(BlueprintReadWrite)
	float longitudinalAcceleration;

	UPROPERTY(BlueprintReadOnly)
	float lateralAcceleration;
	
private:
	const float mass = 1450.0f; // mass in Kg
	const float centreOfMassHeight = 0.48f; // height of the centre of mass above the ground in meters
	const float wheelSeparation = 2.457f; // measured in meters
	const float frontWheelOffset = 0.957f;
	const float rearWheelOffset = 1.5f;
	const float frontTrackWidth = 1.6f;
	const float rearTrackWidth = 1.56f;
	const float rollCentreHeight = 0.1f;
	const float frontSuspensionStiffness = 120000.0f; // suspension in N/m
	const float rearSuspensionStiffness = 200000.0f;

	float CalculateLoadChangeFromLateralForce(float lateralForce, bool forFrontAxel);
	float CalculateWheelLoadRatio();
};
