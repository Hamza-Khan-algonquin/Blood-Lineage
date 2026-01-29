// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBoidMovementComponent.h"
#include "RelicRunners/Enemy/Controllers/CommanderController.h"
#include "RelicRunners/Enemy/EnemyPawn.h"

void UEnemyBoidMovementComponent::CalculateDirection()
{
	//set up variables for each of the influncers on the birds direction
	//The unit vector towards average position of all boids
	FVector coherenceVector = { 0,0,0 };
	//The unit vector of the average oppsite direction to all near by birds
	FVector separationVector = { 0,0,0 };
	//The unit vector of the average alignment of all boids
	FVector alignmentVector = { 0,0,0 };

	FVector collisionAvoidanceVector = { 0,0,0 };
	float currentAvoidanceWeight = 0;

	FVector targetVector = { 0,0,0 };

	FVector finalInflunceVector = { 0,0,0 };

	int boidsEffectingSeparation = 0;

	//get a reference to all boids near me
	TArray<APawn*> pOtherBoidsToCheck;
	TArray<APawn*> pOtherBoids;


	if (Commander)
	{
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

					//check if boid is in range for separation
					if (distanceToBoid < SeparationRange)
					{
						//add the oppisite of the direction vector to the other boid to the separation vector
						separationVector -= directionToBoid;
						boidsEffectingSeparation++;
					}
				}
			}

		}

		pOtherBoids = pOtherBoidsToCheck;
	}





	//average everything out
	//UE_LOG(LogTemp, Warning, TEXT("X: %f, Y: %f, Z: %f"), coherenceVector.X, coherenceVector.Y, coherenceVector.Z);
	coherenceVector /= pOtherBoidsToCheck.Num();
	coherenceVector -= GetOwner()->GetActorLocation();
	coherenceVector.Normalize();
	//UE_LOG(LogTemp, Warning, TEXT("X: %f, Y: %f, Z: %f"), coherenceVector.X, coherenceVector.Y, coherenceVector.Z);
	//UE_LOG(LogTemp, Warning, TEXT("X: %f, Y: %f, Z: %f"), alignmentVector.X, alignmentVector.Y, alignmentVector.Z);
	alignmentVector /= pOtherBoidsToCheck.Num();
	alignmentVector.Normalize();
	//UE_LOG(LogTemp, Warning, TEXT("X: %f, Y: %f, Z: %f"), alignmentVector.X, alignmentVector.Y, alignmentVector.Z);
	separationVector /= boidsEffectingSeparation;
	separationVector.Normalize();
	//UE_LOG(LogTemp, Warning, TEXT("X: %f, Y: %f, Z: %f"), separationVector.X, separationVector.Y, separationVector.Z);
	if (Commander->TargetPlayer)
	{
		targetVector = Commander->TargetPlayer->GetActorLocation() - GetOwner()->GetActorLocation();
		targetVector.Normalize();
	}


	//factor in the weights and add them up to get the final direction vector
	finalInflunceVector = (coherenceVector * CoherenceWeight) + (separationVector * SeparationWeight) + (alignmentVector * AlignmentWeight);
	float totalWeight = (CoherenceWeight + SeparationWeight + AlignmentWeight);

	if (Commander->TargetPlayer)
	{
		finalInflunceVector += (targetVector * TargetWeight);
		totalWeight += TargetWeight;
	}

	finalInflunceVector /= totalWeight;

	//calculate the force to add then add it

	//APawn* pawn = StaticCastPtr<APawn, AActor>(GetOwner());

	FVector turnSpeed = (finalInflunceVector * MaxSpeed) - CurrentVelocity;

	CurrentVelocity = turnSpeed.GetClampedToMaxSize(MaxSpeed);
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
		float angle = directionToBoid.Dot(CurrentVelocity) / (CurrentVelocity.Length() * directionToBoid.Length());

		return angle <= FeildOfView;
	}

	return false;
}

const FVector UEnemyBoidMovementComponent::GetVelocity()
{
	return CurrentVelocity;
}
