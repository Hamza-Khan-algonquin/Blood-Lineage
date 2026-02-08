// Fill out your copyright notice in the Description page of Project Settings.


#include "CommanderController.h"
#include "Perception/AIPerceptionComponent.h"
#include "RelicRunners/Director System/Director.h"
#include "RelicRunners/Enemy/EnemyPawn.h"
#include "RelicRunners/Enemy/EnemyLODComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "RelicRunners/Enemy/EnemyBoidMovementComponent.h"

ACommanderController::ACommanderController()
{
	MaxNumberOfEnemies = 5;
	MaxLifeTimeNumberOfEnemies = 20;
	CurrentLifeTimeNumberOfEnemies = 0;
	SpawnRadius = 1000.f;
	ActiveDistanceFromPlayer = 10000.f;
	AttackDistanceFromPlayer = 8000.f;
	EnemiesSetToLowestLOD = false;
	TimeToAttack = 1;
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

	if (players.Num() <= 0 || !pawn)
	{
		return;
	}

	

	UpdateLODs(players, pawn->GetActorLocation());
	AttackCheck(players);
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

TArray<AEnemyPawn*> ACommanderController::GetPawnEnemies()
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
	newEnemy->MovementComponent->Commander = this;
	AddPawnEnemy(newEnemy);
}

void ACommanderController::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			ADirector* Director = ADirector::GetInstance(this);
			if (Director)
			{
				TArray<APawn*> players = Director->GetPlayers();
				for (int i = 0; i < players.Num(); i++)
				{
					if (players[i])
					{
						RemainingTimeToAttack.Add(players[i], TimeToAttack);

						break;
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Commander Controller: Missing Director in World!"));
			}
		});
}

void ACommanderController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);


}

void ACommanderController::UpdateLODs(TArray<APawn*> players, FVector refernceLocation)
{
	float closestDistance = FVector::Dist(players[0]->GetActorLocation(), refernceLocation);

	for (int i = 1; i < players.Num(); i++)
	{
		float newDistance = FVector::Dist(players[i]->GetActorLocation(), refernceLocation);

		if (closestDistance > newDistance)
		{
			closestDistance = newDistance;
		}
	}

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

void ACommanderController::AttackCheck(TArray<APawn*> players)
{
	//loop through all players and check to see if they are available to be attack through the member varaible map and it to array

	//loop through all enemies

		//check what the close player to it and if it is in target radius

		//check to see if it is the closest enemy

			//set it as the enemy to attack this time in a map with the key be the players



	//loop through the array of players

		//check the map to get for the player keyed to a enemy

		//command the enemy to attack

	//rest the remaining time
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
