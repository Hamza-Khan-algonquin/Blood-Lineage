// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBoidMovementComponent.h"
#include "RelicRunners/Enemy/Controllers/CommanderController.h"
#include "RelicRunners/Enemy/EnemyPawn.h"
#include "NavigationSystem.h"

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
	OrbitWeight = 1.f;
	Velocity = { 0,0,0 };
	NavMeshClampExtent = FVector(50.f, 50.f, 100.f);
}

void UEnemyBoidMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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


	
	if (Commander->TargetPlayer)
	{
		targetDirection = Commander->TargetPlayer->GetActorLocation() - GetOwner()->GetActorLocation();
		float distanceToTarget = targetDirection.Size();
		distanceFromTargetRadius = distanceToTarget - TargetRadius;

		if (FocuseMode == EBoidFocuseMode::Velocity && distanceToTarget <= FocuseOnTargetDistance)
		{
			FocuseMode = EBoidFocuseMode::Target;
		}
		else if (FocuseMode == EBoidFocuseMode::Target && distanceToTarget > FocuseOnTargetDistance)
		{
			FocuseMode = EBoidFocuseMode::Velocity;
		}
		
		currentSeparationDistance = (MaxSeperationRange - MinSeperationRange) * FMath::Clamp(distanceFromTargetRadius / (DistanceForMaxSeperation - distanceFromTargetRadius), 0, 1) + MinSeperationRange;
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
	if (Commander->TargetPlayer)
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

		targetDirection.Normalize();
		FVector orbitVector = FVector::CrossProduct(targetDirection, FVector::UpVector);
		finalInflunceVector += (orbitVector * OrbitWeight);
	}


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

	if (newVelocity.Size2D() >= MinVelocityChange)
	{
		Velocity = newVelocity;
	}
	if (Velocity.Size2D() < MinMoveVelocity)
	{
		//Velocity = FVector::ZeroVector;
	}
	//Velocity = (finalInflunceVector * MaxSpeed).GetClampedToMaxSize(MaxSpeed);
	UpdateComponentVelocity();
}

void UEnemyBoidMovementComponent::CalculateMoveToDirection(float delta)
{
	if (!Commander || !Commander->TargetPlayer)
	{
		return;
	}
	
	FVector targetDirection;
	FVector desiredVelocity = FVector::ZeroVector;
	//just define as a big number to get ride of a warning
	float distanceFromTargetRadius = 100000;

	targetDirection = Commander->TargetPlayer->GetActorLocation() - GetOwner()->GetActorLocation();

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
	FVector desiredMovement = Velocity * DeltaTime;
	FVector finalMovement = FVector::Zero();
	FHitResult hit;

	ClampToNavMesh(GetOwner()->GetActorLocation() + desiredMovement, finalMovement);

	GetOwner()->SetActorLocation(finalMovement);

	//fix if we have time
	/*SafeMoveUpdatedComponent(finalMovement, GetOwner()->GetActorRotation(), false, hit);

	if (hit.IsValidBlockingHit())
	{
		SlideAlongSurface(finalMovement, 1.f - hit.Time, hit.Normal, hit);
	}*/
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
	switch (FocuseMode)
	{
		case EBoidFocuseMode::Velocity:
			GetOwner()->SetActorRotation(Velocity.GetSafeNormal2D().Rotation());
			break;
		case EBoidFocuseMode::Target:
			GetOwner()->SetActorRotation((Commander->TargetPlayer->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal2D().Rotation());
			break;
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
	if (MovementMode == EBoidMovementMode::Disabled || !Commander || !Commander->TargetPlayer)
	{
		return false;
	}

	float distanceToTarget = (Commander->TargetPlayer->GetActorLocation() - GetOwner()->GetActorLocation()).Size();

	if (MovementMode == EBoidMovementMode::Boid)
	{
		return distanceToTarget - TargetRadius <= TargetDeadZone;
	}
	else 
	{
		return distanceToTarget <= DirectMoveToAcceptanceRadius;
	}

	return false;
}
