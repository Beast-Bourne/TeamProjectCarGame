// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Vehicle.generated.h"

UCLASS()
class TEAMPROJECTCARGAME_API AVehicle : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AVehicle();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Body")
	UStaticMeshComponent* CarBody;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	USceneComponent* FL_SuspensionMount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	USceneComponent* FR_SuspensionMount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	USceneComponent* RL_SuspensionMount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	USceneComponent* RR_SuspensionMount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	USceneComponent* FL_SuspensionRest;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	USceneComponent* FR_SuspensionRest;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	USceneComponent* RL_SuspensionRest;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	USceneComponent* RR_SuspensionRest;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire Mesh")
	UStaticMeshComponent* FL_TireMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire Mesh")
	UStaticMeshComponent* FR_TireMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire Mesh")
	UStaticMeshComponent* RL_TireMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire Mesh")
	UStaticMeshComponent* RR_TireMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rim Mesh")
	UStaticMeshComponent* FL_RimMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rim Mesh")
	UStaticMeshComponent* FR_RimMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rim Mesh")
	UStaticMeshComponent* RL_RimMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rim Mesh")
	UStaticMeshComponent* RR_RimMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Properties")
	bool bDebugDraw{false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float SuspensionRestDistance{ 50.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float FrontSuspensionStrength{ 100.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float RearSuspensionStrength{ 150.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float Damper{ 1.0f };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	float GetWheelRadius(UStaticMeshComponent* WheelMesh);

	// Variables

	float WheelRadius;
	FVector TireVelocity;
	float SteerInput;
	float AccelerationInput;
	
	float SuspensionForce;
	float SuspensionMaxLength;
	float SuspensionRayHitDistance;
	float SuspensionCurrentLength{};
	float SuspensionPreviousLength{};

	float FR_SuspensionForce;
	float FR_SuspensionOffset;
	FVector FR_SuspensionDirection;
	float FR_SpringVelocity;

	float FL_SuspensionForce;
	float FL_SuspensionOffset;
	FVector FL_SuspensionDirection;
	float FL_SpringVelocity;

	float RR_SuspensionForce;
	float RR_SuspensionOffset;
	FVector RR_SuspensionDirection;
	float RR_SpringVelocity;

	float RL_SuspensionForce;
	float RL_SuspensionOffset;
	FVector RL_SuspensionDirection;
	float RL_SpringVelocity;

	float TireGripFactor = 0.8f;
	float TireMass = 10.0f;
	float MaxSteeringAngle = 30.0f;
	float SteeringInterpSpeed = 5.0f;
	float CarTopSpeed = 2000000.0f;

	bool carIsGrounded;
	FVector WheelRelativeLocation;


	void SuspensionCast(USceneComponent* Wheel, UStaticMeshComponent* WheelMesh, USceneComponent* SuspensionRest, float SuspensionStrength, float WheelLoad, bool DebugDraw);
	void ApplyAccelerationForce(USceneComponent* Wheel, UStaticMeshComponent* WheelMesh, FVector ResultantForce, FVector Velocity);
	void Debug();
	bool LineTrace(FVector StartLocation, FVector EndLocation, FHitResult& OutHitResult, bool bDrawDebug = false) const;
	bool SweepTrace(FVector StartLocation, FVector EndLocation, FHitResult& OutHitResult, bool bDrawDebug, FRotator Rotation) const;

	UFUNCTION(BlueprintCallable)
	void RunSimulationFrame(float FR_WheelLoad, float FL_WheelLoad, float RR_WheelLoad, float RL_WheelLoad, FVector Velocity, FVector AngularVelocity, bool carIsGrounded);

};

