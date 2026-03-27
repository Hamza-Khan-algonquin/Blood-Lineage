// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnZone.h"
#include <NavigationSystem.h>
#include <NavMesh/NavMeshBoundsVolume.h>


FVector ASpawnZone::GetRandomPosition()
{
	//Get the actor location and the radius to be spawning in
	FVector zonePosition = GetActorLocation();
	float radiusToUse = Radius;

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

	//get a random point on the navmesh
	if (NavSys->GetRandomPointInNavigableRadius(zonePosition, radiusToUse, randomPoint))
	{
		return randomPoint.Location;
	}
	else if (NavSys->ProjectPointToNavigation(zonePosition, randomPoint, FVector(radiusToUse)))
	{
		FVector newPosition = randomPoint.Location;
		if (NavSys->GetRandomPointInNavigableRadius(newPosition, radiusToUse - FVector::Dist(newPosition, zonePosition), randomPoint))
		{
			return randomPoint.Location;
		}
	}


	//if a point was not found log a error and try to spawn a zero zero
	UE_LOG(LogTemp, Error, TEXT("could not find random spawn point on navmesh"));

	return FVector::ZeroVector;
}
