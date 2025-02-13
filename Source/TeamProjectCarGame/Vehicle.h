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

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Mesh")
	UStaticMeshComponent* FL_WheelMeshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Mesh")
	UStaticMeshComponent* FR_WheelMeshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Mesh")
	UStaticMeshComponent* RL_WheelMeshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel Mesh")
	UStaticMeshComponent* RR_WheelMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Properties")
	bool bDebugDraw{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float WheelRadius{ 50.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float SuspensionLength{ 70.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float Stiffness{ 1000.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float Damping{ 3000.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float RestLength{ 50.0f };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Variables
	FVector SuspensionForce;
	float SuspensionMaxLength;
	float SuspensionRayHitDistance;
	float SuspensionCurrentLength{};
	float SuspensionPreviousLength{};

	void SuspensionCast(USceneComponent* Wheel);
	void Debug();
	bool LineTrace(FVector StartLocation, FVector EndLocation, FHitResult& OutHitResult, bool bDrawDebug = false) const;

};
