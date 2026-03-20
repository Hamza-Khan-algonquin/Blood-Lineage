// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBoidMovementComponent.h"
#include "RelicRunners/Enemy/Controllers/CommanderController.h"
#include "RelicRunners/Enemy/EnemyPawn.h"
#include "NavigationSystem.h"
#include <Net/UnrealNetwork.h>
#include "DrawDebugHelpers.h"
#include "NavMesh/RecastNavMesh.h"
#include "Detour/DetourNavMeshQuery.h"
#include "RelicRunners/Flow Field/FlowFieldComponent.h"

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
	MaxStepNormal = 0.1;
	IsKnockedBack = false;
	InterpDelay = 0.1f;
	MaxClientLocationDeviation = 200.f;
	AvgReplicationDelta = 0.1f;
	DistanceFromWallToLoseTargetWeight = 100;
	SetIsReplicatedByDefault(true);
}

void UEnemyBoidMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwnerRole() != ROLE_Authority)
	{
		ClientSideEntityInterpolation(DeltaTime);

		return;
	}
	
	if (IsKnockedBack)
	{
		return;
	}

	//kill plain
	if (UpdatedComponent->GetComponentLocation().Z > 1000000)
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
		//update the server move data for clients to interpolate 
		ServerMoveData = FMoveData(UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentRotation(), 0.0f);
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

	float distanceToTarget = 0;

	FVector targetDirection;
	FVector targetLocation;
	//just define as a big number to get ride of a warning
	float distanceFromTargetRadius = 100000;

	APawn* target = Commander->GetCurrentTarget();

	//int boidsEffectingSeparation = 0;

	//get a reference to all boids near me
	TArray<APawn*> pOtherBoidsToCheck;
	TArray<APawn*> pOtherBoids;

	if (IsAffectiveByBundleOfJoy)
	{
		targetDirection = BundleOfJoyLocation - GetOwner()->GetActorLocation();
		
	}
	else if(IsValid(target))
	{
		targetLocation = target->GetActorLocation();
		targetDirection = targetLocation - GetOwner()->GetActorLocation();
		distanceToTarget = targetDirection.Size();
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

	if (ActiveNavMesh)
	{
		FNavLocation navLocation;
		float closeWallDist = DistanceToStartAvoidingWalls;

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
					closeWallDist = 0;
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

					if (distanceToWall < closeWallDist)
					{
						closeWallDist = distanceToWall;
					}
				}

				finalInflunceVector += direction * strength * WallAvoidanceWeight;
				//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + direction * strength * WallAvoidanceWeight * 100, FColor::Cyan, false, 0.1, 0, 3);
			}
		}


		if (!IsAffectiveByBundleOfJoy && IsValid(target))
		{
			if (FMath::Abs(distanceFromTargetRadius) > TargetDeadZone)
			{
				float targetMult = FMath::Clamp((distanceFromTargetRadius), -MaxTargetDistanceMultiplier, MaxTargetDistanceMultiplier);
				FVector StartLocation = UpdatedComponent->GetComponentLocation();
				FVector EndLocation = targetLocation;
				FVector HitLocation;
				bool didHit = ActiveNavMesh->NavMeshRaycast(ActiveNavMesh, StartLocation, EndLocation, HitLocation, ActiveNavMesh->GetDefaultQueryFilter(), this);

				UFlowFieldComponent* flowField = target->GetComponentByClass<UFlowFieldComponent>();

				if (didHit && IsValid(flowField) && flowField->GetFieldRadius() >= distanceToTarget)
				{
					targetVector = flowField->GetDirection(UpdatedComponent->GetComponentLocation(), { 100,100,100 }) * targetMult;
				}
				else
				{
					targetVector = targetDirection.GetSafeNormal() * targetMult;
				}
			}

			finalInflunceVector += (targetVector * TargetWeight) * FMath::Clamp(closeWallDist / DistanceFromWallToLoseTargetWeight, 0, 1);
			//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + targetVector * TargetWeight * 100, FColor::Emerald, false, 0.1, 0, 3);
		}
	}

	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + inVector.GetSafeNormal() * 100, FColor::Cyan, false, 0.1, 0, 3);

	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + finalInflunceVector.GetSafeNormal() * 50, FColor::Black, false, 0.1, 0, 3);

	//factor in the weights and add them up to get the final direction vector
	finalInflunceVector += (coherenceVector * CoherenceWeight) + (separationVector * SeparationWeight) + (alignmentVector * AlignmentWeight);
	
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
	
	FVector targetVector;
	FVector desiredVelocity = FVector::ZeroVector;
	APawn* target = Commander->GetCurrentTarget();

	targetVector = target->GetActorLocation() - GetOwner()->GetActorLocation();

	if (targetVector.Size() > DirectMoveToAcceptanceRadius)
	{
		FVector StartLocation = UpdatedComponent->GetComponentLocation();
		FVector EndLocation = target->GetActorLocation();
		FVector HitLocation;
		bool didHit = ActiveNavMesh->NavMeshRaycast(ActiveNavMesh, StartLocation, EndLocation, HitLocation, ActiveNavMesh->GetDefaultQueryFilter(), this);

		UFlowFieldComponent* flowField = target->GetComponentByClass<UFlowFieldComponent>();

		if (didHit && IsValid(flowField) && flowField->GetFieldRadius() >= FVector::Dist(StartLocation, EndLocation))
		{
			targetVector = flowField->GetDirection(UpdatedComponent->GetComponentLocation(), { 100,100,100 });
		}
		else
		{
			targetVector = targetVector.GetSafeNormal();
		}

		desiredVelocity = targetVector.GetSafeNormal() * MaxSpeed;
		FVector turnSpeed = (desiredVelocity - Velocity).GetClampedToMaxSize(MaxTurnSpeed * delta);

		Velocity = (Velocity + turnSpeed).GetClampedToMaxSize(MaxSpeed);
	}
	else
	{
		Velocity = desiredVelocity;
	}

	
	UpdateComponentVelocity();
}

void UEnemyBoidMovementComponent::MoveInDirection(float DeltaTime)
{
	if (Velocity.ContainsNaN())
	{
		Velocity = FVector::ZeroVector;
	}

	FVector startLocation = GetActorLocation();
	FVector desiredMovement = Velocity * DeltaTime;
	FVector finalMovement = startLocation + desiredMovement;
	FHitResult hit;

	//ClampToNavMesh(GetOwner()->GetActorLocation() + desiredMovement, finalMovement);

	FNavLocation navLocation;
	UNavigationSystemV1* navSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FVector offset;
	FVector origin;
	GetOwner()->GetActorBounds(true, origin, offset, false);
	offset.X = 0;
	offset.Y = 0;
	offset.Z -= 10;
	
	//FVector actualExtent = GetActorFeetLocation() + offset;//NavMeshClampExtent + offset;

	//DrawDebugLine(GetWorld(), GetActorLocation(), finalMovement, FColor::Red, false, 0.05, 0, 3);

	if (ActiveNavMesh)//navSys)
	{
		if (navSys->ProjectPointToNavigation(finalMovement, navLocation, {100,100,200}))
		{
			FNavLocation navOutLocation;
			ActiveNavMesh->FindMoveAlongSurface(navLocation, finalMovement, navOutLocation, ActiveNavMesh->GetDefaultQueryFilter(), this);
			finalMovement = navOutLocation.Location + offset - startLocation;
		}
		
		/*if (navSys->ProjectPointToNavigation(finalMovement, navLocation, NavMeshClampExtent))
		{
			FVector newPosition = navLocation.Location + offset;// + offset.Z;
			finalMovement = newPosition - startLocation;
			
			//EdgeAvoidanceVector = FVector::Zero();
			LastValidPosition = newPosition;
		}
		else if (navSys->ProjectPointToNavigation(startLocation, navLocation, FVector(100,100,200)))
		{
			FVector newPosition = navLocation.Location + offset;// + offset.Z;
			finalMovement = newPosition - startLocation;
			//finalMovement = ((navLocation.Location + offset) - startLocation).GetSafeNormal() * Velocity.Length();
			//finalMovement.Z = navLocation.Location.Z + offset.Z;
		}
		else
		{
			return;
		}*/
	}
	

	SafeMoveUpdatedComponent(finalMovement, GetOwner()->GetActorRotation(), true, hit);

	if (hit.IsValidBlockingHit())
	{
		SlideAlongSurface(finalMovement, 1.f - hit.Time, hit.Normal, hit);
		//Velocity = FVector::VectorPlaneProject(Velocity, hit.Normal);
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

void UEnemyBoidMovementComponent::OnRep_ServerMoveData()
{
	ServerMoveData.TimeStamp = GetWorld()->GetTimeSeconds();

/*	if (MoveDataBuffer.Num() > 2)
	{
		float clientDelta = ServerMoveData.TimeStamp - MoveDataBuffer.Last().TimeStamp;

		//Finds the delay between the server and client and lerps towards it to make sure the interpolation is smooth when the delay changes. This is just simpluare than calculating the true average delay.
		AvgReplicationDelta = FMath::Lerp(AvgReplicationDelta, clientDelta, 0.05f);

		TargetInterpDelay = FMath::Clamp(AvgReplicationDelta * 1.5f, 0.05f, 0.5f);
	}*/

	MoveDataBuffer.Add(ServerMoveData);

	if (MoveDataBuffer.Num() > 6)
	{
		MoveDataBuffer.RemoveAt(0);
	}

}

void UEnemyBoidMovementComponent::ClientSideEntityInterpolation(float delta)
{
	if (MoveDataBuffer.Num() < 2)
	{
		return;
	}

	float clientDelta = MoveDataBuffer.Last().TimeStamp - MoveDataBuffer[MoveDataBuffer.Num() - 2].TimeStamp;

	AvgReplicationDelta = FMath::FInterpTo(AvgReplicationDelta, clientDelta, delta, 1.0f);

	TargetInterpDelay = FMath::Clamp(AvgReplicationDelta * 1.5f, 0.05f, 0.5f);

	InterpDelay = FMath::FInterpTo(InterpDelay, TargetInterpDelay, delta, 5.0f);

	float clientRenderTime = GetWorld()->GetTimeSeconds() - (InterpDelay);

	//loop through the move data buffer and find the two move data that the client render time is in between
	for (int i = 0; i < MoveDataBuffer.Num() - 1; i++)
	{
		if (MoveDataBuffer[i].TimeStamp <= clientRenderTime && MoveDataBuffer[i + 1].TimeStamp >= clientRenderTime)
		{
			float denominator = (MoveDataBuffer[i + 1].TimeStamp - MoveDataBuffer[i].TimeStamp);

			if (denominator < KINDA_SMALL_NUMBER)
			{
				continue;
			}

			float interpAlpha = (clientRenderTime - MoveDataBuffer[i].TimeStamp) / denominator;

			//lerp!!!!!!!!!!!!!!!!!!!!
			FVector interpLocation = FMath::Lerp(MoveDataBuffer[i].Location, MoveDataBuffer[i + 1].Location, interpAlpha);
			FRotator interpRotation = FMath::Lerp(MoveDataBuffer[i].Rotation, MoveDataBuffer[i + 1].Rotation, interpAlpha);

			//check if the interpolation is too far from the last move data to prevent interpolation from going crazy when there is a big delay between the server and client
			if (FVector::Dist(interpLocation, MoveDataBuffer.Last().Location) > MaxClientLocationDeviation)
			{
				UpdatedComponent->SetWorldLocationAndRotation(MoveDataBuffer.Last().Location, interpRotation);
				//clear so we don't get rubber banding effect
				MoveDataBuffer.Empty();
			}
			else
			{
				UpdatedComponent->SetWorldLocationAndRotation(interpLocation, interpRotation);
			}

			return;
		}
	}

	if (clientRenderTime > MoveDataBuffer.Last().TimeStamp)
	{
		UpdatedComponent->SetWorldLocationAndRotation(MoveDataBuffer.Last().Location, MoveDataBuffer.Last().Rotation);
	}
}

void UEnemyBoidMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UEnemyBoidMovementComponent, ServerMoveData);
}