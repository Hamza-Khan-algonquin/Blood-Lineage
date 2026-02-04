// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "EnemyBoidMovementComponent.generated.h"

class ACommanderController;

UCLASS()
class RELICRUNNERS_API UEnemyBoidMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<ACommanderController> Commander;
protected:
	UPROPERTY(EditAnywhere)
	float VisualRange;
	UPROPERTY(EditAnywhere)
	float FeildOfView;
	UPROPERTY(EditAnywhere)
	float SeparationRange;
	UPROPERTY(EditAnywhere)
	float MaxTurnSpeed;
	UPROPERTY(EditAnywhere)
	float MaxSpeed;
	UPROPERTY(EditAnywhere)
	float TargetRadius;

	UPROPERTY(EditAnywhere)
	float CoherenceWeight;
	UPROPERTY(EditAnywhere)
	float SeparationWeight;
	UPROPERTY(EditAnywhere)
	float AlignmentWeight;
	UPROPERTY(EditAnywhere)
	float TargetWeight;
	UPROPERTY(EditAnywhere)
	float MaxTargetDistanceWeight;

	UPROPERTY(EditAnywhere)
	float OrbitWeight;
	
	UPROPERTY(EditAnywhere)
	FVector NavMeshClampExtent;

public:
	UEnemyBoidMovementComponent();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	//calculate the direction the boid should be move in and update the velocity
	void CalculateDirection(float delta);
	//clamp the direction to move onto the navmesh
	bool ClampToNavMesh(const FVector& DesiredMovement, FVector& out);
	//move in a given direction
	void MoveInDirection(float DeltaTime);
	//move the enemy onto the ground
	void GroundSnap();
	//check to see if another boid is in range and feild of view
	bool CanSeeOtherBoid(APawn* otherBoid, FVector& directionToBoid, float& distanceToBoid);
	const FVector GetVelocity();
};
