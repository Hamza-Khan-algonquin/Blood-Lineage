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
struct FMoveDataWithTimeStamp
{
	GENERATED_BODY()
	UPROPERTY()
	FVector Location = FVector::ZeroVector;
	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;
	UPROPERTY()
	float TimeStamp = -1;
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
	UPROPERTY(Replicated)
	int32 ServerSideID = -1;
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
	float EnemyCharacterAvoidanceDistance;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float DistanceToStartAvoidingWalls;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float DistanceFromWallToLoseTargetWeight;

	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float CoherenceWeight;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float SeparationWeight;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float AlignmentWeight;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float TargetWeight;
	UPROPERTY(EditAnywhere, Category = "Boid Behavior")
	float EnemyCharacterAvoidanceWeight;
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

	UPROPERTY()
	TArray<FMoveDataWithTimeStamp> MoveDataBuffer;

	
	UPROPERTY(EditAnywhere, Category = "NetWorking Movement")
	float InterpDelay;
	//The max distance the client is allowed to be from the server location before it snaps to it. 
	UPROPERTY(EditAnywhere, Category = "NetWorking Movement")
	float MaxClientLocationDeviation;
	UPROPERTY()
	float AvgReplicationDelta;
	UPROPERTY()
	float TargetInterpDelay;

	
public:
	UEnemyBoidMovementComponent();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	void UpdateMovement(float delta);
	void UpdateMovementInfo(float delta);

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
	void AddToMoveDataBuffer(FVector newLocation, FRotator newRotation);

	UFUNCTION(NetMulticast, Reliable)
	void ClearMoveBuffer();

	UFUNCTION()
	void ClientSideEntityInterpolation(float delta);
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
};
