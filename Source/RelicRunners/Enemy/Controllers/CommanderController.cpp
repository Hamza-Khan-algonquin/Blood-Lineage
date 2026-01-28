// Fill out your copyright notice in the Description page of Project Settings.


#include "CommanderController.h"
#include "Perception/AIPerceptionComponent.h"
#include "RelicRunners/Director System/Director.h"
#include "RelicRunners/Enemy/EnemyPawn.h"
#include "RelicRunners/Enemy/EnemyLODComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"

ACommanderController::ACommanderController()
{
	MaxNumberOfEnemies = 5;
	MaxLifeTimeNumberOfEnemies = 20;
	CurrentLifeTimeNumberOfEnemies = 0;
	SpawnRadius = 1000.f;
	ActiveDistanceFromPlayer = 10000.f;
	AttackDistanceFromPlayer = 8000.f;
	EnemiesSetToLowestLOD = false;
}

void ACommanderController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APawn* pawn = GetPawn();
	TArray<APawn*> players;

	if (ADirector* dir = ADirector::GetInstance(this))
	{
		players = dir->GetPlayers();
	}
	else
	{
		return;
	}

	if (players.Num() <= 0 || !pawn)
	{
		return;
	}

	float closeDistance = FVector::Dist(players[0]->GetActorLocation(), pawn->GetActorLocation());

	for (int i = 1; i < players.Num(); i++)
	{
		float newDistance = FVector::Dist(players[i]->GetActorLocation(), pawn->GetActorLocation());

		if (closeDistance > newDistance)
		{
			closeDistance = newDistance;
		}
	}

	UpdateLODs(closeDistance);
}

void ACommanderController::AddPawnEnemy(AEnemyPawn* enemy)
{
	if (!enemy)
	{
		return;
	}

	ControlledEnemies.Add(enemy);
	enemy->LODComponent->SetIsUpdatingLOD(false);
}

TArray<TObjectPtr<AEnemyPawn>> ACommanderController::GetPawnEnemies()
{
	return ControlledEnemies;
}

void ACommanderController::SpawnPawnEnemy()
{
	APawn* pawn = GetPawn();

	if (!pawn)
	{
		return;
	}

	FVector spawnPosition = pawn->GetActorLocation();

	FNavLocation randomPoint;
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

	if (NavSys->GetRandomPointInNavigableRadius(spawnPosition, SpawnRadius, randomPoint))
	{
		spawnPosition = randomPoint.Location;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("could not find random spawn point on navmesh"));
	}


	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	AEnemyPawn* newEnemy = Cast<AEnemyPawn>(GetWorld()->SpawnActor<AEnemyPawn>(PawnEnemyToSpawn, spawnPosition, FRotator::ZeroRotator, SpawnParams));
	AddPawnEnemy(newEnemy);
}

void ACommanderController::BeginPlay()
{
	Super::BeginPlay();
}

void ACommanderController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);


}

void ACommanderController::UpdateLODs(float closestDistance)
{

	if (closestDistance > ActiveDistanceFromPlayer && !EnemiesSetToLowestLOD)
	{
		for (AEnemyPawn* enemy : ControlledEnemies)
		{
			if (enemy && enemy->LODComponent)
			{
				enemy->LODComponent->SetLOD(4);
			}
		}

		PrimaryActorTick.TickInterval = 5.0f;

		EnemiesSetToLowestLOD = true;

	}
	else if(closestDistance <= ActiveDistanceFromPlayer)
	{
		if (EnemiesSetToLowestLOD)
		{
			PrimaryActorTick.TickInterval = 0.2f;
			EnemiesSetToLowestLOD = false;
		}

		for (AEnemyPawn* enemy : ControlledEnemies)
		{
			if (enemy && enemy->LODComponent)
			{
				enemy->LODComponent->UpdateLOD();
			}
		}
	}
}

void ACommanderController::UpdatePlayerTracking(float closestDistance, APawn* ClosetPlayer)
{
	if (ActiveDistanceFromPlayer >= closestDistance)
	{
		//loop through enemies and find the closest ones to the player do a recast to see if they can see the player and if so do the rest
		
		//loop through enemies and set focus to the player

		if (GetMoveStatus() == EPathFollowingStatus::Idle)
		{
			//MoveToActor(ClosetPlayer, 100);
		}
	}
	else
	{
		StopMovement();
	}
}
