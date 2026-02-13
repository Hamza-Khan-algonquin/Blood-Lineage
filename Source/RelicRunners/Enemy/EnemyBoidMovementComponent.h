// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "EnemyBoidMovementComponent.generated.h"

UENUM(BlueprintType)
enum class EBoidMovementMode : uint8
{
	Boid,
	DirectMoveTo,
	Disabled
};

UENUM(BlueprintType)
enum class EBoidFocuseMode : uint8
{
	Velocity,
	Target,
	Disabled
};

class ACommanderController;

UCLASS()
class RELICRUNNERS_API UEnemyBoidMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<ACommanderController> Commander;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float VisualRange;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float FeildOfView;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float DefaultSeparationRange;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float MaxSeperationRange;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float MinSeperationRange;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float DistanceForMaxSeperation;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float MaxTurnSpeed;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float MaxSpeed;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float TargetRadius;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float TargetDeadZone;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float MaxTargetDistanceMultiplier;

	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float CoherenceWeight;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float SeparationWeight;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float AlignmentWeight;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float TargetWeight;
	

	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float OrbitWeight;

	UPROPERTY(EditAnywhere, Category = "Direct Move To Behavior")
	float DirectMoveToAcceptanceRadius;
	
	UPROPERTY(EditAnywhere, Category = "Other")
	FVector NavMeshClampExtent;
	UPROPERTY(EditAnywhere, Category = "Other")
	float FocuseOnTargetDistance;

	EBoidMovementMode MovementMode = EBoidMovementMode::Boid;
	EBoidFocuseMode FocuseMode = EBoidFocuseMode::Velocity;

public:
	UEnemyBoidMovementComponent();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	//calculate the direction the boid should be move in and update the velocity
	void CalculateBoidDirection(float delta);
	//calculate the direction the actor should move in to get to the target and update the velocity
	void CalculateMoveToDirection(float delta);
	//clamp the direction to move onto the navmesh
	bool ClampToNavMesh(const FVector& DesiredMovement, FVector& out);
	//move in a given direction
	void MoveInDirection(float DeltaTime);
	//move the enemy onto the ground
	void GroundSnap();
	void UpdateRotation();
	//check to see if another boid is in range and feild of view
	bool CanSeeOtherBoid(APawn* otherBoid, FVector& directionToBoid, float& distanceToBoid);
	const FVector GetVelocity();
	float GetTotalTargetRadius() { return TargetRadius + TargetDeadZone; }

	virtual void StopMovementImmediately() override;
	void SetMovementMode(EBoidMovementMode newMode) { MovementMode = newMode; }
	EBoidMovementMode GetMovementMode() { return MovementMode; }
	bool IsAtTarget();
};
