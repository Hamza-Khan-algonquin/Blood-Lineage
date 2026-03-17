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
#include "RelicRunners/ObjectPool/ObjectPoolManager.h"

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
	
}

void ACommanderController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	
	ADirector* director = ADirector::GetInstance(this);
	if (director)
	{
		director->OnPlayerAdd.RemoveDynamic(this, &ACommanderController::AddPlayerToTimeToAttack);
		director->OnPlayerRemoved.RemoveDynamic(this, &ACommanderController::RemovePlayerFromTimeToAttack);
	}

	GetWorldTimerManager().ClearTimer(SpawnTimer);
	Super::EndPlay(EndPlayReason);
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

	if (players[0])
	{
		
		float distance; 

		if (!TargetPlayer)
		{
			APawn* closestPlayer = players[0];
			distance = FVector::DistSquared(pawn->GetActorLocation(), players[0]->GetActorLocation());

			for (int i = 1; i < players.Num(); i++)
			{
				float newDistance = FVector::DistSquared(pawn->GetActorLocation(), players[i]->GetActorLocation());
				if (newDistance < distance)
				{
					closestPlayer = players[i];
					distance = newDistance;
				}
			}

			UpdateTarget(closestPlayer);
			TargetChange.Broadcast(TargetPlayer);
			distance = FMath::Sqrt(distance);
			BoidShouldFocusePlayer = false;
		}
		else
		{
			distance = FVector::Distance(pawn->GetActorLocation(), TargetPlayer->GetActorLocation());
			BoidShouldFocusePlayer = distance <= DistanceForBoidsToFocusePlayer;
		}

		IsTargeting = distance <= DistanceToTargetPlayer;

		if (!BTComp && IsTargeting && GetMoveStatus() == EPathFollowingStatus::Idle)
		{
			MoveToActor(TargetPlayer, 25);
		}
		else if (!IsTargeting)
		{
			BoidShouldFocusePlayer = false;
			UpdateTarget(nullptr);
			if (!BTComp || !BTComp->IsRunning())
			{
				StopMovement();
			}
		}
	}
	else
	{
		UpdateTarget(nullptr);
	}

	UpdateLODs(players, pawn->GetActorLocation());
	AttackCheck(players, DeltaTime);
	//SpawmLoop();

	if (CurrentLifeTimeNumberOfEnemies >= MaxLifeTimeNumberOfEnemies && ControlledEnemies.Num() == 0)
	{
		if (!IsValid(BTComp) || !BTComp->IsRunning())
		{
			Destroy();
		}	
	}
}

void ACommanderController::AddPawnEnemy(AEnemyPawn* enemy)
{
	if (!IsValid(enemy) || !IsValid(this))
	{
		return;
	}

	ControlledEnemies.AddUnique(enemy);
	enemy->MovementComponent->Commander = this;
	enemy->OnDied.AddUniqueDynamic(this, &ACommanderController::RemovePawnEnemy);
	enemy->OnDestroyed.AddUniqueDynamic(this, &ACommanderController::RemovePawnEnemy);
	enemy->OnDeactivated.AddUniqueDynamic(this, &ACommanderController::RemovePawnEnemy);
	enemy->LODComponent->SetIsUpdatingLOD(false);
}

void ACommanderController::RemovePawnEnemy(AActor* enemy)
{
	AEnemyPawn* enemyPawn = Cast<AEnemyPawn>(enemy);

	if (!enemyPawn || !ControlledEnemies.Contains(enemyPawn)) return;

	enemyPawn->OnDestroyed.RemoveDynamic(this, &ACommanderController::RemovePawnEnemy);
	enemyPawn->OnDeactivated.RemoveDynamic(this, &ACommanderController::RemovePawnEnemy);

	ControlledEnemies.Remove(enemyPawn);
}

TArray<AEnemyPawn*> ACommanderController::GetPawnEnemies()
{
	return ControlledEnemies;
}

void ACommanderController::SpawnPawnEnemy()
{
	APawn* pawn = GetPawn();
	AObjectPoolManager* pool = AObjectPoolManager::GetInstance(this);

	if (!pawn || !pool)
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


	/*FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;*/

	AEnemyPawn* newEnemy = Cast<AEnemyPawn>(pool->GetActor(PawnEnemyToSpawn));//Cast<AEnemyPawn>(GetWorld()->SpawnActor<AEnemyPawn>(PawnEnemyToSpawn, spawnPosition, FRotator::ZeroRotator, SpawnParams));
	FVector origin;
	FVector bounds;

	newEnemy->GetActorBounds(true, origin, bounds);

	newEnemy->SetActorLocation(spawnPosition + FVector(0, 0, bounds.Z));

	//giving time for the enemy to set
	GetWorldTimerManager().SetTimerForNextTick([this, newEnemy]()
		{
			//newEnemy->SetActorLocation(spawnPosition);
			AddPawnEnemy(newEnemy);
		});
}

void ACommanderController::SpawmLoop()
{
	if (!GetWorld() || GetWorld()->TimeSeconds < 5)
	{
		return;
	}

	for (int i = 0; i < EnemyToSpawnPerLoop && ControlledEnemies.Num() < MaxNumberOfEnemies; i++)
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

void ACommanderController::UpdateTarget_Implementation(APawn* newTarget)
{
	TargetPlayer = newTarget;
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

	if (StartingBehaviorTree)
	{
		RunBehaviorTree(StartingBehaviorTree);
	}

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(SpawnTimer, this, &ACommanderController::SpawmLoop, TimeBetweenSpawnChecks, true);
	}
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
	if (ControlledEnemies.Num() <= 0 || players.Num() <= 0 || !ControlledEnemies[0])
	{
		return;
	}

	for (int i = 0; i < players.Num(); i++)
	{
		APawn* CurrentPlayer = players[i];
		if (!IsValid(CurrentPlayer)) continue;

		AEnemyPawn* closestEnemy = nullptr;
		float closestDistance = TNumericLimits<float>::Max();

		for (int j = 0; j < ControlledEnemies.Num(); j++)
		{
			AEnemyPawn* CurrentEnemy = ControlledEnemies[j];

			if (!IsValid(CurrentEnemy) || !IsValid(CurrentEnemy->MovementComponent)) continue;

			float newDistance = FVector::DistSquared(CurrentPlayer->GetActorLocation(), CurrentEnemy->GetActorLocation());
			if (newDistance < closestDistance && !CurrentEnemy->IsAttacking)
			{
				closestDistance = newDistance;
				closestEnemy = CurrentEnemy;
			}
		}
		if (closestEnemy)
		{
			float trueDistance = FMath::Sqrt(closestDistance);

			if (trueDistance <= closestEnemy->MovementComponent->GetTotalTargetRadius())
			{
				if (RemainingTimeToAttack.Contains(CurrentPlayer) && RemainingTimeToAttack[CurrentPlayer] > 0)
				{
					RemainingTimeToAttack[CurrentPlayer] -= delta;
				}
				else
				{
					UE_LOG(LogTemp, Log, TEXT("Enemy goes in for the kill"));
					closestEnemy->StartAttack();
					RemainingTimeToAttack.FindOrAdd(CurrentPlayer) = TimeToAttack;
				}
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
