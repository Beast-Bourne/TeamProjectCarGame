// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "Vehicle.generated.h"

UCLASS()
class RACINGDRIVE_API AVehicle : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AVehicle();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	UBoxComponent* BoxComponent;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheels")
	USceneComponent* FL_Wheels;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheels")
	USceneComponent* FR_Wheels;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheels")
	USceneComponent* RL_Wheels;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheels")
	USceneComponent* RR_Wheels;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Mesh")
	UStaticMeshComponent* FL_WheelMeshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Mesh")
	UStaticMeshComponent* FR_WheelMeshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Mesh")
	UStaticMeshComponent* RL_WheelMeshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Mesh")
	UStaticMeshComponent* RR_WheelMeshes;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float BaseTurningTorque{250000000};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float ZTorque{0};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float MinTurningClamp{0};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float MaxTurningClamp{10};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float MaxZTorque{2000000};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float MaxTurningSpeed{75};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float MaxSteeringAngle{45};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float MinSteeringAngle{25};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float Speed{};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float SuspensionForce{750000};

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float FL_WheelDurability{100};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float FR_WheelDurability{100};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float RL_WheelDurability{100};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float RR_WheelDurability{100};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float WheelUsage{0.01};

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float DefaultAccelerationMultiplier{};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float AccelerationMultiplier{8000};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float AccelerationInput{0};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float Acceleration{};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float MinAccelerationLerp{0};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float MaxAccelerationLerp{50000};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	float AccelerationLerpAlpha{0.0};

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	bool bCanAccelerate{true};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	bool bBombAffected{false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	bool bIsTurning{false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	bool bAirControl{false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	bool bIsLapping{false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	bool bUsingNitrous{false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car Properties")
	bool bIsSuspensionActive{false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Properties")
	bool bDebugDraw{false};

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	bool LineTrace(FVector StartLocation, FVector EndLocation, FHitResult& OutHitResult, bool bDrawDebug = false) const;
	void SuspensionCast(USceneComponent* Wheel);
	float SuspensionCastInterpolationSpeed = 1.0;

};
