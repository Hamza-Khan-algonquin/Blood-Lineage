// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnZone.h"
#include <NavigationSystem.h>
#include <NavMesh/NavMeshBoundsVolume.h>


FVector ASpawnZone::GetRandomPosition()
{
	//Get the actor location and the radius to be spawning in
	FVector zonePosition = GetActorLocation();
	float radiusToUse = Radius;
	FVector finalPoint = FVector::ZeroVector;
	bool foundPoint = false;
	//check to if bouds for spawning was set and if so set the variable to use it information
	/*if (SpawningBounds != nullptr)
	{
		FVector boxExtent;
		SpawningBounds->GetActorBounds(false, zonePosition, boxExtent);
		radiusToUse = boxExtent.GetMax();
	}*/

	//A random point of the navmesh
	FNavLocation randomPoint;
	//Getting the current nav mesh system in the world instance
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

	//20 checks and if we don't find a point near player then we will spawn at zero zero
	for (int i = 0; i < 20; i++)
	{
		//get a random point on the navmesh
		if (NavSys->GetRandomPointInNavigableRadius(zonePosition, radiusToUse, randomPoint))
		{
			finalPoint = randomPoint.Location;
			foundPoint = true;
		}
		else if (NavSys->ProjectPointToNavigation(zonePosition, randomPoint, FVector(radiusToUse)))
		{
			FVector newPosition = randomPoint.Location;
			if (NavSys->GetRandomPointInNavigableRadius(newPosition, radiusToUse - FVector::Dist(newPosition, zonePosition), randomPoint))
			{
				finalPoint = randomPoint.Location;
				foundPoint = true;
			}
		}

		if(foundPoint)
		{
			//check to make sure it in the budget for spawning
			if (FVector::Dist(finalPoint, zonePosition) >= MinDistanceFromPlayer)
			{
				break;
			}
			else
			{
				foundPoint = false;
			}
		}
	}

	if(foundPoint)
	{
		return finalPoint;
	}

	//if a point was not found log a error and try to spawn a zero zero
	UE_LOG(LogTemp, Warning, TEXT("could not find random spawn point on navmesh"));

	return FVector::ZeroVector;
}
