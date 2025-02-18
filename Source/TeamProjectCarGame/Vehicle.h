// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
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
	float SuspensionStrength{ 1000.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float Damper{ 3000.0f };

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
	FVector SpringDirection;
	FVector TireVelocity;
	
	float SuspensionForce;
	float SuspensionMaxLength;
	float SuspensionRayHitDistance;
	float SuspensionCurrentLength{};
	float SuspensionPreviousLength{};

	void SuspensionCast(USceneComponent* Wheel, UStaticMeshComponent* WheelMesh, USceneComponent* SuspensionRest);
	void Debug();
	bool LineTrace(FVector StartLocation, FVector EndLocation, FHitResult& OutHitResult, bool bDrawDebug = false) const;
	bool SweepTrace(FVector StartLocation, FVector EndLocation, FHitResult& OutHitResult, bool bDrawDebug) const;

};
