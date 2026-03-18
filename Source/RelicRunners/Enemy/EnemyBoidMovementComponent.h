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

USTRUCT()
struct FMoveData
{
	GENERATED_BODY()
	UPROPERTY()
	FVector Location;
	UPROPERTY()
	FRotator Rotation;
	UPROPERTY()
	float TimeStamp;
};

class ACommanderController;
class ARecastNavMesh;

UCLASS()
class RELICRUNNERS_API UEnemyBoidMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<ACommanderController> Commander;
	
	bool IsKnockedBack;
	bool IsAffectiveByBundleOfJoy = false;
	FVector BundleOfJoyLocation;
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
	float DistanceToStartAvoidingWalls;

	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float CoherenceWeight;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float SeparationWeight;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float AlignmentWeight;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float TargetWeight;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float EdgeAvoidanceWeight;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float WallAvoidanceWeight;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float SlowDownDistance;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float MinSlowDownPercentage;

	//UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	//float OrbitWeight;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float MinMoveVelocity;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float MinVelocityChange;

	UPROPERTY(EditAnywhere, Category = "Direct Move To Behavior")
	float DirectMoveToAcceptanceRadius;
	
	UPROPERTY(EditAnywhere, Category = "Other")
	FVector NavMeshClampExtent;
	UPROPERTY(EditAnywhere, Category = "Other")
	FVector WallAvoidanceExtent;
	UPROPERTY(EditAnywhere, Category = "Other")
	float FocuseOnTargetDistance;
	UPROPERTY(EditAnywhere, Category = "Other")
	float StepHeight;
	UPROPERTY(EditAnywhere, Category = "Other")
	//the normal at which the movement stop trying to slide along a surface and instead does a step check
	float MaxStepNormal;

	UPROPERTY(VisibleAnywhere)
	EBoidMovementMode MovementMode = EBoidMovementMode::Boid;
	EBoidFocuseMode FocuseMode = EBoidFocuseMode::Velocity;

	FVector EdgeAvoidanceVector = FVector::Zero();
	FVector LastValidPosition = FVector::Zero();
	FVector LastEdgeVector = FVector::Zero();
	FVector LastWallAvoidanceDirection = FVector::Zero();

	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float BundleOfJoyWeight;

	UPROPERTY()
	ARecastNavMesh* ActiveNavMesh;

	UPROPERTY(ReplicatedUsing = OnRep_ServerMoveData)
	FMoveData ServerMoveData;

	UPROPERTY()
	TArray<FMoveData> MoveDataBuffer;

	//This is client side only and should not be replicated
	//How far back in time the client should interpolate the server data and is based on a rough-ish average time between server updates using lerp. 
	UPROPERTY(EditAnywhere, Category = "NetWorking Movement")
	float InterpDelay;
	//The max distance the client is allowed to be from the server location before it snaps to it. 
	UPROPERTY(EditAnywhere, Category = "NetWorking Movement")
	float MaxClientLocationDeviation;
public:
	UEnemyBoidMovementComponent();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	//calculate the direction the boid should be move in and update the velocity
	void CalculateBoidDirection(float delta);
	//calculate the direction the actor should move in to get to the target and update the velocity
	void CalculateMoveToDirection(float delta);
	//UFUNCTION(NetMulticast, Reliable)
	//move in a given direction
	void MoveInDirection(float DeltaTime);
	void UpdateRotation();
	//check to see if another boid is in range and feild of view
	bool CanSeeOtherBoid(APawn* otherBoid, FVector& directionToBoid, float& distanceToBoid);
	const FVector GetVelocity();
	float GetTotalTargetRadius() { return TargetRadius + TargetDeadZone; }

	virtual void StopMovementImmediately() override;
	void SetMovementMode(EBoidMovementMode newMode) { MovementMode = newMode; }
	EBoidMovementMode GetMovementMode() { return MovementMode; }
	bool IsAtTarget();
	void InitializeNavMeshPointer();


	UFUNCTION()
	void OnRep_ServerMoveData();
	UFUNCTION()
	void ClientSideEntityInterpolation();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
};
