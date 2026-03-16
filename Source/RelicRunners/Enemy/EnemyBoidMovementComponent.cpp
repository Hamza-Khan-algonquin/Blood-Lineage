// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBoidMovementComponent.h"
#include "RelicRunners/Enemy/Controllers/CommanderController.h"
#include "RelicRunners/Enemy/EnemyPawn.h"
#include "NavigationSystem.h"
#include <Net/UnrealNetwork.h>
#include "DrawDebugHelpers.h"
#include "NavMesh/RecastNavMesh.h"
#include "Detour/DetourNavMeshQuery.h"

UEnemyBoidMovementComponent::UEnemyBoidMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	VisualRange = 1000.f;
	FeildOfView = 180.f;
	DefaultSeparationRange = 200.f;
	MaxSeperationRange = 800.f;
	MinSeperationRange = 200.f;
	DistanceForMaxSeperation = 2000.f;
	MaxTurnSpeed = 800.0f;
	MaxSpeed = 600.f;
	TargetRadius = 300.f;
	CoherenceWeight = 1.f;
	SeparationWeight = 1.5f;
	AlignmentWeight = 1.f;
	TargetWeight = 2.f;
	MaxTargetDistanceMultiplier = 5.f;
	Velocity = { 0,0,0 };
	NavMeshClampExtent = FVector(50.f, 50.f, 100.f);
	WallAvoidanceExtent = FVector(100.f, 100.f, 300.f);
	EdgeAvoidanceWeight = 1;
	MaxStepNormal = 0.1;
	IsKnockedBack = false;
	SetIsReplicatedByDefault(true);
}

void UEnemyBoidMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwnerRole() != ROLE_Authority || IsKnockedBack)
	{
		return;
	}

	switch(MovementMode)
	{ 	
	case EBoidMovementMode::Boid:
		CalculateBoidDirection(DeltaTime);
		break;
	case EBoidMovementMode::DirectMoveTo:
		CalculateMoveToDirection(DeltaTime);
		break;
	}

	if (MovementMode != EBoidMovementMode::Disabled)
	{
		MoveInDirection(DeltaTime);
		UpdateRotation();
		//GroundSnap();
	}
}

void UEnemyBoidMovementComponent::CalculateBoidDirection(float delta)
{
	if (!Commander)
	{
		return;
	}
	//set up variables for each of the influncers on the birds direction
	//The unit vector towards average position of all boids
	FVector coherenceVector = { 0,0,0 };
	//The unit vector of the average oppsite direction to all near by birds
	FVector separationVector = { 0,0,0 };
	float currentSeparationDistance = DefaultSeparationRange;

	//The unit vector of the average alignment of all boids
	FVector alignmentVector = { 0,0,0 };

	FVector collisionAvoidanceVector = { 0,0,0 };
	float currentAvoidanceWeight = 0;

	FVector targetVector = { 0,0,0 };

	FVector finalInflunceVector = { 0,0,0 };

	FVector targetDirection;
	//just define as a big number to get ride of a warning
	float distanceFromTargetRadius = 100000;

	//int boidsEffectingSeparation = 0;

	//get a reference to all boids near me
	TArray<APawn*> pOtherBoidsToCheck;
	TArray<APawn*> pOtherBoids;

	if (IsAffectiveByBundleOfJoy)
	{
		targetDirection = BundleOfJoyLocation - GetOwner()->GetActorLocation();
		
	}
	else if(APawn * target = Commander->GetCurrentTarget())
	{
		targetDirection = target->GetActorLocation() - GetOwner()->GetActorLocation();
		float distanceToTarget = targetDirection.Size();
		distanceFromTargetRadius = distanceToTarget - TargetRadius;

		if (FocuseMode == EBoidFocuseMode::Velocity && distanceToTarget <= FocuseOnTargetDistance && Commander->BoidShouldFocusePlayer)
		{
			FocuseMode = EBoidFocuseMode::Target;
		}
		else if (FocuseMode == EBoidFocuseMode::Target && distanceToTarget > FocuseOnTargetDistance)
		{
			FocuseMode = EBoidFocuseMode::Velocity;
		}

		if (Commander->BoidShouldFocusePlayer)
		{
			currentSeparationDistance = (MaxSeperationRange - MinSeperationRange) * FMath::Clamp(distanceFromTargetRadius / (DistanceForMaxSeperation - distanceFromTargetRadius), 0, 1) + MinSeperationRange;
		}
		else
		{
			currentSeparationDistance = MaxSeperationRange;
		}
	}

	TArray<AEnemyPawn*> boidsInFlock = Commander->GetPawnEnemies();

	for (int i = 0; i < boidsInFlock.Num(); i++)
	{
		if (boidsInFlock[i])
		{
			UEnemyBoidMovementComponent* boidMovement = boidsInFlock[i]->MovementComponent;
			FVector directionToBoid;
			float distanceToBoid;

			if (boidMovement && CanSeeOtherBoid(boidsInFlock[i], directionToBoid, distanceToBoid))
			{
				pOtherBoidsToCheck.Add(boidsInFlock[i]);

				//add up the position of the boid for the coherence 
				coherenceVector += boidsInFlock[i]->GetActorLocation();

				//add the alignment of the boid
				alignmentVector += boidMovement->GetVelocity();

				//check if boid is in range for separation. distanceToBoid > 0 make sure we are not going into inf
				if (distanceToBoid < currentSeparationDistance && distanceToBoid > 0 && boidsInFlock[i]->MovementComponent != this)
				{
					float linear = distanceToBoid / currentSeparationDistance;


					//add the oppisite of the direction vector to the other boid to the separation vector
					separationVector -= directionToBoid.GetSafeNormal() * (1 - (linear * linear * (3 - 2 * linear)));
					//boidsEffectingSeparation++;
				}
			}
		}
	}

	pOtherBoids = pOtherBoidsToCheck;
	
	//average everything out
	//UE_LOG(LogTemp, Warning, TEXT("X: %f, Y: %f, Z: %f"), coherenceVector.X, coherenceVector.Y, coherenceVector.Z);
	coherenceVector /= pOtherBoidsToCheck.Num();
	coherenceVector -= GetOwner()->GetActorLocation();
	coherenceVector.Normalize();
	//UE_LOG(LogTemp, Warning, TEXT("X: %f, Y: %f, Z: %f"), coherenceVector.X, coherenceVector.Y, coherenceVector.Z);
	//UE_LOG(LogTemp, Warning, TEXT("X: %f, Y: %f, Z: %f"), alignmentVector.X, alignmentVector.Y, alignmentVector.Z);
	alignmentVector /= pOtherBoidsToCheck.Num();
	//alignmentVector -= Velocity;
	alignmentVector.Normalize();
	//UE_LOG(LogTemp, Warning, TEXT("X: %f, Y: %f, Z: %f"), alignmentVector.X, alignmentVector.Y, alignmentVector.Z);
	//if (boidsEffectingSeparation != 0)
	//{
	//	separationVector /= boidsEffectingSeparation;
	//	//separationVector.Normalize();
	//}
	
	//UE_LOG(LogTemp, Warning, TEXT("X: %f, Y: %f, Z: %f"), separationVector.X, separationVector.Y, separationVector.Z);
	if (IsAffectiveByBundleOfJoy)
	{
		finalInflunceVector += targetDirection.GetSafeNormal() * BundleOfJoyWeight;
		//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + targetVector.GetSafeNormal() * BundleOfJoyWeight, FColor::Purple, false, 0.1, 0, 3);
	}
	else if (Commander->GetCurrentTarget())
	{
		if (FMath::Abs(distanceFromTargetRadius) > TargetDeadZone)
		{
			targetVector = targetDirection.GetSafeNormal() * FMath::Clamp((distanceFromTargetRadius), -MaxTargetDistanceMultiplier, MaxTargetDistanceMultiplier);
		}
		else
		{
			targetVector = FVector::Zero();

			//target direction is already normalized so don't need to worry about the dividing step in projection
			targetDirection.Normalize();

			//project alignmentVector on to targetDirection and than subtract that from aligment to get a perpendicular angle to targetDirection that magnitude is the differents in between the project and aligment vectors
			//Desmos has a 3d graphing calculator now. yaaaaa
			alignmentVector -= targetDirection * FVector::DotProduct(alignmentVector, targetDirection);
			alignmentVector.Normalize();

			coherenceVector -= targetDirection * FVector::DotProduct(coherenceVector, targetDirection);
			coherenceVector.Normalize();


			alignmentVector = FVector::Zero();
			coherenceVector = FVector::Zero();
		}

		finalInflunceVector += (targetVector * TargetWeight);

		/*targetDirection.Normalize();
		FVector orbitVector = FVector::CrossProduct(targetDirection, FVector::UpVector);
		finalInflunceVector += (orbitVector * OrbitWeight);*/
		
	}

/*	FVector edgeVector = FVector::Zero();
	FVector inVector = (LastValidPosition - GetActorLocation());
	inVector.Z = 0;
	inVector = inVector.GetSafeNormal();
	if (!EdgeAvoidanceVector.IsNearlyZero())
	{
		edgeVector = -EdgeAvoidanceVector.GetSafeNormal();

		
		FVector leftDirection = FVector::CrossProduct(FVector::UpVector, edgeVector);
		FVector goal = finalInflunceVector.GetSafeNormal();

		//"|" is the dot product. I just felt like
		if ((goal | leftDirection) > 0)
		{
			edgeVector = leftDirection.GetSafeNormal();
		}
		else
		{
			edgeVector = -leftDirection.GetSafeNormal();
		}
		
		if (FVector::DotProduct(edgeVector, LastEdgeVector) < 0.f)
		{
			edgeVector = -edgeVector;
		}
	}
	
	if (!LastEdgeVector.IsNearlyZero() || !edgeVector.IsZero())
	{
		edgeVector = FMath::VInterpTo(LastEdgeVector, edgeVector, delta, 5);
		LastEdgeVector = edgeVector;
		finalInflunceVector += (edgeVector + inVector * 0.5).GetSafeNormal() * finalInflunceVector.Length();//EdgeAvoidanceWeight;
	}*/


	if (ActiveNavMesh)
	{
		FNavLocation navLocation;

		if (ActiveNavMesh->ProjectPoint(GetActorLocation(), navLocation, WallAvoidanceExtent))
		{
			NavNodeRef polyRef = navLocation.NodeRef;
			TArray<FNavigationPortalEdge> wallPoints;
	
			ActiveNavMesh->GetPolyWallSegments(polyRef, ActiveNavMesh->GetDefaultQueryFilter(), this, wallPoints);

			for (const FNavigationPortalEdge& wallPoint : wallPoints)
			{
				FVector closestPointOnWall = FMath::ClosestPointOnSegment(navLocation.Location, wallPoint.Left, wallPoint.Right);
				float distanceToWall = FVector::Dist(navLocation.Location, closestPointOnWall);
				FVector direction;
				float strength;
				if (distanceToWall > DistanceToStartAvoidingWalls)
				{
					continue;
				}

				if (!FMath::IsNearlyZero(distanceToWall))
				{
					strength = FMath::Clamp((1 - distanceToWall / DistanceToStartAvoidingWalls), 0, 1);
					direction = (navLocation.Location - closestPointOnWall).GetSafeNormal2D();
					
				}
				else
				{
					strength = 1;
					FVector edgeDirection = (wallPoint.Left - wallPoint.Right).GetSafeNormal();

					//it the cross product. Don't believe me go look at the body of FVector::CrossProduct. I just felt like being a dick to anyone who tries to read this mess
					direction = edgeDirection ^ FVector::UpVector;

					FVector polyCenter;
					ActiveNavMesh->GetPolyCenter(wallPoint.ToRef, polyCenter);

					if ((direction | polyCenter - wallPoint.GetMiddlePoint()) < 0)
					{
						direction *= -1;
					}
				}

				finalInflunceVector += direction * strength * WallAvoidanceWeight;
				//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + direction * strength * WallAvoidanceWeight * 100, FColor::Cyan, false, 0.1, 0, 3);
			}
		}
	}

	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + inVector.GetSafeNormal() * 100, FColor::Cyan, false, 0.1, 0, 3);

	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + finalInflunceVector.GetSafeNormal() * 50, FColor::Black, false, 0.1, 0, 3);

	//factor in the weights and add them up to get the final direction vector
	finalInflunceVector += (coherenceVector * CoherenceWeight) + (separationVector * SeparationWeight) + (alignmentVector * AlignmentWeight) /*+ (EdgeAvoidanceVector.GetSafeNormal() * EdgeAvoidanceWeight)*/;
	
	//float totalWeight = (CoherenceWeight + SeparationWeight + AlignmentWeight);

	

	//if (Commander->TargetPlayer)
	//{
		//totalWeight += TargetWeight;
	//}

	//finalInflunceVector /= totalWeight;
	//finalInflunceVector.Normalize();

	FVector desiredVelocity = finalInflunceVector.GetSafeNormal() * MaxSpeed;

	FVector turnSpeed = (desiredVelocity - Velocity).GetClampedToMaxSize(MaxTurnSpeed * delta);

	FVector newVelocity = (Velocity + turnSpeed).GetClampedToMaxSize(MaxSpeed);

	if (Commander->TargetPlayer && Commander->GetCurrentTarget() == Commander->TargetPlayer && !IsAffectiveByBundleOfJoy)
	{
		float slowDownPercentage = FMath::Clamp(FVector::Distance(Commander->GetCurrentTarget()->GetActorLocation() / SlowDownDistance, GetOwner()->GetActorLocation()), MinSlowDownPercentage, 1);
		newVelocity *= slowDownPercentage;
	}

	/*if (newVelocity.Size2D() >= MinVelocityChange)
	{
		Velocity = newVelocity;
	}*/

	Velocity = newVelocity;

	//Velocity = (finalInflunceVector * MaxSpeed).GetClampedToMaxSize(MaxSpeed);
	UpdateComponentVelocity();

	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + Velocity, FColor::Red, false, 0.05, 0, 3);
}

void UEnemyBoidMovementComponent::CalculateMoveToDirection(float delta)
{
	if (!Commander || !Commander->GetCurrentTarget())
	{
		return;
	}
	
	FVector targetDirection;
	FVector desiredVelocity = FVector::ZeroVector;
	//just define as a big number to get ride of a warning
	float distanceFromTargetRadius = 100000;

	targetDirection = Commander->GetCurrentTarget()->GetActorLocation() - GetOwner()->GetActorLocation();

	if (targetDirection.Size() > DirectMoveToAcceptanceRadius)
	{
		desiredVelocity = targetDirection.GetSafeNormal() * MaxSpeed;
		FVector turnSpeed = (desiredVelocity - Velocity).GetClampedToMaxSize(MaxTurnSpeed * delta);

		Velocity = (Velocity + turnSpeed).GetClampedToMaxSize(MaxSpeed);
	}
	else
	{
		Velocity = desiredVelocity;
	}

	
	UpdateComponentVelocity();
}

bool UEnemyBoidMovementComponent::ClampToNavMesh(const FVector& DesiredMovement, FVector& out)
{
	FNavLocation navLocation;
	UNavigationSystemV1* navSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FVector offset;
	FVector standIn;
	GetOwner()->GetActorBounds(false, standIn, offset, false);
	offset.X = 0;
	offset.Y = 0;
	FVector actualExtent = GetActorFeetLocation() + offset;//NavMeshClampExtent + offset;

	if (navSys && navSys->ProjectPointToNavigation(DesiredMovement, navLocation, NavMeshClampExtent))
	{
		out = navLocation.Location + FVector(0,0, offset.Z - 5.5);
		return true;
	}

	return false;
}

void UEnemyBoidMovementComponent::MoveInDirection(float DeltaTime)
{
	FVector startLocation = GetActorLocation();
	FVector desiredMovement = Velocity * DeltaTime;
	FVector finalMovement = startLocation + desiredMovement;
	FHitResult hit;

	//ClampToNavMesh(GetOwner()->GetActorLocation() + desiredMovement, finalMovement);

	FNavLocation navLocation;
	UNavigationSystemV1* navSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FVector offset;
	FVector origin;
	GetOwner()->GetActorBounds(false, origin, offset, false);
	offset.X = 0;
	offset.Y = 0;
	
	//FVector actualExtent = GetActorFeetLocation() + offset;//NavMeshClampExtent + offset;

	//DrawDebugLine(GetWorld(), GetActorLocation(), finalMovement, FColor::Red, false, 0.05, 0, 3);

	if (navSys)
	{
		if (navSys->ProjectPointToNavigation(finalMovement, navLocation, NavMeshClampExtent))
		{
			FVector newPosition = navLocation.Location + offset;// + offset.Z;
			//move it down slightly to trigger the slide along the surface, as when this is not down, the y jitter up and down sometimes
			finalMovement = navLocation.Location - startLocation;
			
			//EdgeAvoidanceVector = FVector::Zero();
			LastValidPosition = newPosition;
		}
		else if (navSys->ProjectPointToNavigation(startLocation, navLocation, FVector(100,100,200)))
		{
			finalMovement = (navLocation.Location + offset - startLocation).GetSafeNormal() * Velocity.Length();
			//finalMovement.Z = navLocation.Location.Z + offset.Z;
		}
		else
		{
			return;
		}
	}
	
	

	/*FVector offset;
	FVector standIn;
	GetOwner()->GetActorBounds(false, standIn, offset, false);

	FNavLocation navLocation;
	UNavigationSystemV1* navSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (navSys->ProjectPointToNavigation(finalMovement, navLocation, GetActorFeetLocation() + offset))
	{
		finalMovement.Z = navLocation.Location.Z; 
	}
	else
	{
		return;
	}

	finalMovement -= GetOwner()->GetActorLocation();*/

	//GetOwner()->SetActorLocation(finalMovement);

	//fix if we have time
	SafeMoveUpdatedComponent(finalMovement, GetOwner()->GetActorRotation(), true, hit);

	if (hit.IsValidBlockingHit())
	{
		if (FMath::Abs(hit.Normal.Z) < MaxStepNormal)
		{
			FHitResult stepHit;
			
			FVector stepDown = UpdatedComponent->GetComponentLocation();

			SafeMoveUpdatedComponent(FVector(0,0,StepHeight), GetOwner()->GetActorRotation(), true, stepHit);

			if (!stepHit.IsValidBlockingHit())
			{
				

				FHitResult secondHit;
				SafeMoveUpdatedComponent(finalMovement * (1.f - hit.Time), GetOwner()->GetActorRotation(), true, secondHit);

				if (secondHit.IsValidBlockingHit())
				{
					SlideAlongSurface(finalMovement, 1.f - secondHit.Time, secondHit.Normal, secondHit);
					Velocity = FVector::VectorPlaneProject(Velocity, secondHit.Normal);
				}

				stepDown = stepDown - UpdatedComponent->GetComponentLocation();

				FHitResult FinalHit;
				//moving back down so that it only move up the amount need
				SafeMoveUpdatedComponent(FVector(0, 0, -stepDown.Z), GetOwner()->GetActorRotation(), true, FinalHit);

			/*	if (FinalHit.IsValidBlockingHit())
				{
					GetOwner()->SetActorLocation(FinalHit.Location + offset);
				}*/

			}
		}
		else
		{
			SlideAlongSurface(finalMovement, 1.f - hit.Time, hit.Normal, hit);
			Velocity = FVector::VectorPlaneProject(Velocity, hit.Normal);
		}
	}
}

void UEnemyBoidMovementComponent::GroundSnap()
{
	FVector start = GetOwner()->GetActorLocation();
	FVector end = start - FVector(0, 0, 1000.f);
	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(GetOwner());
	bool bHit = GetWorld()->LineTraceSingleByChannel(hit, start, end, ECC_WorldStatic, params);
	if (bHit)
	{
		FVector newLocation = hit.Location;
		FVector offset;
		FVector standIn;
		GetOwner()->GetActorBounds(false, standIn, offset, false);
		GetOwner()->SetActorLocation(newLocation + FVector(0,0, offset.Z));
	}
}

void UEnemyBoidMovementComponent::UpdateRotation()
{
	if(FocuseMode == EBoidFocuseMode::Velocity)
	{
		GetOwner()->SetActorRotation(Velocity.GetSafeNormal2D().Rotation());
	}
	else if (FocuseMode == EBoidFocuseMode::Target && Commander)
	{
		APawn* target = Commander->GetCurrentTarget();

		if (!target)
		{
			return;
		}
				
		GetOwner()->SetActorRotation((Commander->GetCurrentTarget()->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal2D().Rotation());
	}
}

bool UEnemyBoidMovementComponent::CanSeeOtherBoid(APawn* otherBoid, FVector& directionToBoid, float& distanceToBoid)
{
	//ingore the boids I can't see
	FVector otheractorlocation = otherBoid->GetActorLocation();
	FVector thisActor = GetOwner()->GetActorLocation();
	FVector distanceToBoidVector = otheractorlocation - thisActor;
	distanceToBoid = distanceToBoidVector.Length();
	directionToBoid = distanceToBoidVector.GetSafeNormal();

	if (otherBoid == GetOwner())
	{
		return true;
	}

	if (distanceToBoid <= VisualRange)
	{
		float angle = directionToBoid.Dot(Velocity) / (Velocity.Length() * directionToBoid.Length());
		return angle <= FeildOfView;
	}

	return false;
}

const FVector UEnemyBoidMovementComponent::GetVelocity()
{
	if (Velocity.ContainsNaN())
	{
		Velocity = FVector::ZeroVector;
	}

	return Velocity;
}

void UEnemyBoidMovementComponent::StopMovementImmediately()
{
	Super::StopMovementImmediately();
	MovementMode = EBoidMovementMode::Disabled;
}

bool UEnemyBoidMovementComponent::IsAtTarget()
{
	if (MovementMode == EBoidMovementMode::Disabled || !Commander || !Commander->GetCurrentTarget())
	{
		return false;
	}

	float distanceToTarget = (Commander->GetCurrentTarget()->GetActorLocation() - GetOwner()->GetActorLocation()).Size();

	if (MovementMode == EBoidMovementMode::Boid)
	{
		return distanceToTarget - TargetRadius <= TargetDeadZone;
	}

	return distanceToTarget <= DirectMoveToAcceptanceRadius;
	

	//return false;
}

void UEnemyBoidMovementComponent::InitializeNavMeshPointer()
{
	UNavigationSystemV1* navSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!navSys)
	{
		return;
	}

	ARecastNavMesh* recastNavMesh = Cast<ARecastNavMesh>(navSys->GetDefaultNavDataInstance());
	if (!recastNavMesh)
	{
		return;
	}

	ActiveNavMesh = recastNavMesh;
}
