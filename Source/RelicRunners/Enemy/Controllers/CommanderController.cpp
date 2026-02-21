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
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacter.h"

ACommanderController::ACommanderController()
{
	MaxNumberOfEnemies = 5;
	MaxLifeTimeNumberOfEnemies = 20;
	CurrentLifeTimeNumberOfEnemies = 0;
	SpawnRadius = 1000.f;
	ActiveDistanceFromPlayer = 10000.f;
	DistanceToTargetPlayer = 5000.f;
	DistanceForBoidsToFocusePlayer = 1000;
	EnemiesSetToLowestLOD = false;
	TimeToAttack = 1;
	bReplicates = true;
	SetReplicateMovement(true);
}

ACommanderController::~ACommanderController()
{
	ADirector* director = ADirector::GetInstance(this);
	if (director)
	{
		director->OnPlayerAdd.RemoveDynamic(this, &ACommanderController::AddPlayerToTimeToAttack);
		director->OnPlayerRemoved.RemoveDynamic(this, &ACommanderController::RemovePlayerFromTimeToAttack);
	}
}

void ACommanderController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority())
	{
		return;
	}

	APawn* pawn = GetPawn();
	TArray<APawn*> players;
	UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent);

	if (ADirector* dir = ADirector::GetInstance(this))
	{
		players = dir->GetPlayers();
	}

	if (players.Num() <= 0 || !pawn)
	{
		return;
	}

	if (!BTComp || !BTComp->IsRunning() && players[0])
	{
		
		float distance; 

		if (!TargetPlayer)
		{
			APawn* closestPlayer = players[0];
			distance = FVector::DistSquared(pawn->GetActorLocation(), players[0]->GetActorLocation());

			for (int i = 1; i < players.Num(); i++)
			{
				float newDistance = FVector::DistSquared(pawn->GetActorLocation(), players[i]->GetActorLocation());
				if (newDistance > distance)
				{
					closestPlayer = players[i];
					distance = newDistance;
				}
			}

			TargetPlayer = closestPlayer;
			distance = FMath::Sqrt(distance);
			BoidShouldFocusePlayer = false;
		}
		else
		{
			distance = FVector::Distance(pawn->GetActorLocation(), TargetPlayer->GetActorLocation());
			BoidShouldFocusePlayer = distance <= DistanceForBoidsToFocusePlayer;
		}

		IsTargeting = distance <= DistanceToTargetPlayer;

		if (IsTargeting && GetMoveStatus() == EPathFollowingStatus::Idle)
		{
			MoveToActor(TargetPlayer, 25);
		}
		else if (!IsTargeting)
		{
			BoidShouldFocusePlayer = false;
			TargetPlayer = nullptr;
			StopMovement();
		}
	}
	else
	{
		TargetPlayer = nullptr;
	}

	UpdateLODs(players, pawn->GetActorLocation());
	AttackCheck(players, DeltaTime);
	SpawmLoop();

	if (CurrentLifeTimeNumberOfEnemies >= MaxLifeTimeNumberOfEnemies && ControlledEnemies.Num() == 0 && !Cast<AEnemyCharacter>(GetPawn()))
	{
		Destroy();
		//GetPawn()->Destroy();
	}
}

void ACommanderController::AddPawnEnemy(AEnemyPawn* enemy)
{
	if (!enemy)
	{
		return;
	}


	ControlledEnemies.Add(enemy);
	enemy->MovementComponent->Commander = this;
	enemy->OnDied.AddDynamic(this, &ACommanderController::RemovePawnEnemy);
	enemy->OnDestroyed.AddDynamic(this, &ACommanderController::RemovePawnEnemy);
	enemy->OnDeactivated.AddDynamic(this, &ACommanderController::RemovePawnEnemy);
	enemy->LODComponent->SetIsUpdatingLOD(false);
}

void ACommanderController::RemovePawnEnemy(AActor* enemy)
{
	AEnemyPawn* enemyPawn = Cast<AEnemyPawn>(enemy);
	if (!enemyPawn)
	{
		return;
	}
	ControlledEnemies.Remove(enemyPawn);
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
	
	//giving time for the enemy to set
	GetWorldTimerManager().SetTimerForNextTick([this, newEnemy]()
		{
			
			AddPawnEnemy(newEnemy);
		});
}

void ACommanderController::SpawmLoop()
{
	for (int i = 0; i < MaxNumberOfEnemies; i++)
	{
		if (CurrentLifeTimeNumberOfEnemies < MaxLifeTimeNumberOfEnemies)
		{
			SpawnPawnEnemy();
			CurrentLifeTimeNumberOfEnemies++;
		}
		else
		{
			break;
		}
	}
}

inline void ACommanderController::AddPlayerToTimeToAttack(APawn* player)
{
	RemainingTimeToAttack.Add(player, TimeToAttack);
}

inline void ACommanderController::RemovePlayerFromTimeToAttack(APawn* player)
{
	RemainingTimeToAttack.Remove(player);
}

APawn* ACommanderController::GetCurrentTarget()
{
	if (BoidShouldFocusePlayer && TargetPlayer)
	{
		return TargetPlayer;
	}

	return GetPawn();
}

void ACommanderController::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			ADirector* director = ADirector::GetInstance(this);
			if (director)
			{
				TArray<APawn*> players = director->GetPlayers();
				for (int i = 0; i < players.Num(); i++)
				{
					if (players[i])
					{
						AddPlayerToTimeToAttack(players[i]);
						break;
					}
				}

				director->OnPlayerAdd.AddDynamic(this, &ACommanderController::AddPlayerToTimeToAttack);
				director->OnPlayerRemoved.AddDynamic(this, &ACommanderController::RemovePlayerFromTimeToAttack);
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

void ACommanderController::AttackCheck(TArray<APawn*> players, float delta)
{
	if (ControlledEnemies.Num() <= 0 || players.Num() <= 0 || ControlledEnemies[0])
	{
		return;
	}

	//loop through all players
	for (int i = 0; i < players.Num(); i++)
	{
		//check what the close enemy to it and if it is in the enemies target radius
		float closestDistance = FVector::DistSquared(players[i]->GetActorLocation(), ControlledEnemies[0]->GetActorLocation());
		AEnemyPawn* closestEnemy = ControlledEnemies[0];

		for (int j = 1; j < ControlledEnemies.Num(); j++)
		{
			float newDistance = FVector::DistSquared(players[i]->GetActorLocation(), ControlledEnemies[j]->GetActorLocation());
			if (closestDistance > newDistance)
			{
				closestDistance = newDistance;
				closestEnemy = ControlledEnemies[j];
			}
		}

		float trueDistance = FMath::Sqrt(closestDistance);
		//check to see if the enemy is in their target Radius
		if (trueDistance <= closestEnemy->MovementComponent->GetTotalTargetRadius())
		{
			if (RemainingTimeToAttack.Contains(players[i]) && RemainingTimeToAttack[players[i]] > 0)
			{
				RemainingTimeToAttack[players[i]] -= delta;
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Enemy goes in for the kill"));
				closestEnemy->StartAttack();
				RemainingTimeToAttack[players[i]] = TimeToAttack;
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
