// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
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

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Mesh")
	UStaticMeshComponent* FL_WheelMeshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Mesh")
	UStaticMeshComponent* FR_WheelMeshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Mesh")
	UStaticMeshComponent* RL_WheelMeshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Mesh")
	UStaticMeshComponent* RR_WheelMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Properties")
	bool bDebugDraw{false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float SuspensionRestDistance{ 50.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float SuspensionStrength{ 20000.0f };
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

	float FL_SuspensionForce;
	float FL_SuspensionOffset;
	FVector FL_SuspensionDirection;

	float RR_SuspensionForce;
	float RR_SuspensionOffset;
	FVector RR_SuspensionDirection;

	float RL_SuspensionForce;
	float RL_SuspensionOffset;
	FVector RL_SuspensionDirection;

	float TireGripFactor = 0.8f;
	float TireMass = 10.0f;
	float MaxSteeringAngle = 30.0f;
	float SteeringInterpSpeed = 5.0f;
	float CarTopSpeed = 2000000.0f;


	void SuspensionCast(USceneComponent* Wheel, UStaticMeshComponent* WheelMesh, USceneComponent* SuspensionRest);
	void ApplySteeringForce(USceneComponent* Wheel, UStaticMeshComponent* WheelMesh);
	void Steer(float Value);
	void Accelerate(float Value);
	void ApplyAccelerationForce(USceneComponent* Wheel, UStaticMeshComponent* WheelMesh);
	void RotateSteeringWheels(float DeltaTime);
	void Debug();
	bool LineTrace(FVector StartLocation, FVector EndLocation, FHitResult& OutHitResult, bool bDrawDebug = false) const;
	bool SweepTrace(FVector StartLocation, FVector EndLocation, FHitResult& OutHitResult, bool bDrawDebug) const;

};

