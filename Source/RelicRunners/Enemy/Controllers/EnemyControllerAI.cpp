/************************************************************************************
 * Author:   Tristan Anglin
 * Year:     2025
 *
 * Notice:
 *   This source code is the intellectual property of Tristan Anglin.
 *   It is provided solely for use within:
 *     - The Relic Runners personal project
 *     - The Almost A Dozen project
 *
 *   Any use, distribution, or modification outside of these projects
 *   is strictly prohibited without explicit written permission.
 *
 *   © 2025 Tristan Anglin. All rights reserved.
 ************************************************************************************/

#include "EnemyControllerAI.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"

void AEnemyControllerAI::BeginPlay()
{
	Super::BeginPlay();

	// For now, get first player pawn as target
	PlayerActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	// Start with roaming
	MoveToRandomLocation();
}

void AEnemyControllerAI::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!PlayerActor) return;

	float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), PlayerActor->GetActorLocation());

	if (DistanceToPlayer <= SightRadius)
	{
		// Chase player
		MoveToActor(PlayerActor, AttackRange);

		if (DistanceToPlayer <= AttackRange)
		{
			// Attack logic here (play montage, apply damage, etc.)
		}
	}
	else
	{
		// Roam if reached roam target
		if (FVector::Dist(GetPawn()->GetActorLocation(), RoamLocation) < 50.f)
		{
			MoveToRandomLocation();
		}
	}
}

void AEnemyControllerAI::MoveToRandomLocation()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSystem)
	{
		FNavLocation Result;
		if (NavSystem->GetRandomPointInNavigableRadius(ControlledPawn->GetActorLocation(), RoamRadius, Result))
		{
			RoamLocation = Result.Location;
			MoveToLocation(RoamLocation);
		}
	}
}