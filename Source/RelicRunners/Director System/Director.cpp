#include "Director.h"
#include "SpawnPoint.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ADirector::ADirector() 
{  
	MaxSpawnPointActivisionDistance = 10000;
	SpawnWait = 20;
	PrimaryActorTick.bCanEverTick = true;
	Tags.Add("Director");
	SetReplicates(true);
}

// Called when the game starts or when spawned
void ADirector::BeginPlay()
{
	Super::BeginPlay();
	//make sure we are the server
	if (HasAuthority())
	{
		//sort enemies by their weights for the weight sums algorithm used when picking who to spawn
		DefaultSpawningCards.Sort([](const USpawnCard& A, const USpawnCard& B)
			{
				return A.SpawnWeight < B.SpawnWeight;
			});

		//reseting some varialbes
		SpawnTimer = 0;
	}
	//if we are not the server make sure we don't tick
	else if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		PrimaryActorTick.bCanEverTick = false;
	}
}

void ADirector::RemoveEnemy(AActor* enemy)
{
	//cast the actor point as a APawn then check to make sure it when through before destroying it
	APawn* enemyPtr = static_cast<APawn*>(enemy);

	if (enemyPtr)
	{
		EnemiesInWorld.Remove(enemyPtr);
	}
}

void ADirector::AddEnemy(AActor* enemy)
{
	//cast the actor point as a APawn then check to make sure it when through before adding it
	APawn* enemyPtr = static_cast<APawn*>(enemy);

	if (enemyPtr)
	{
		EnemiesInWorld.Add(enemyPtr);
	}
}

bool ADirector::ContainsEnemy(AActor* enemy)
{
	//cast the actor point as a APawn then check to make sure it when through before checking it
	APawn* enemyPtr = static_cast<APawn*>(enemy);

	if (enemyPtr)
	{
		return EnemiesInWorld.Contains(enemyPtr);
	}

	return false;
}

TArray<USpawnCard*> ADirector::GetDefaultSpawnCards()
{
	return DefaultSpawningCards;
}

TArray<APawn*> ADirector::GetPlayers()
{
	return Players;
}

void ADirector::AddPlayer(APawn* newPlayer)
{
	//check to make sure we don't already have the new player and if so add them
	if (!Players.Contains(newPlayer))
	{
		Players.Add(newPlayer);
	}
}

void ADirector::SpawnCheck()
{
	//Setting up the array
	TArray<AActor*> spawnPoints;

	//grab all spawnpoints in the scene
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnPoint::StaticClass(), spawnPoints);

	for (int i = 0; i < spawnPoints.Num(); i++)
	{
		//cast the actor as spawn points
		ASpawnPoint* point = static_cast<ASpawnPoint*>(spawnPoints[i]);

		for (APawn* player : Players)
		{
			if (!player) continue;

			//check to see if the player is close enough and the spawn point is not at max capacity
			if (FVector::Dist2D(player->GetActorLocation(), point->GetActorLocation()) < MaxSpawnPointActivisionDistance && !point->IsAtMaxCapacity())
			{
				point->SpawnEnemies();
			}
		}
	}
}

// Called every frame
void ADirector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//make sure we are on the server
	if (HasAuthority())
	{
		//check to see if it is time for a spawn check
		if (SpawnTimer <= 0)
		{
			SpawnCheck();
			SpawnTimer = SpawnWait;
		}
		else
		{
			SpawnTimer -= DeltaTime;

			if (SpawnTimer < 0)
			{
				SpawnTimer = 0;
			}
		}
	}
}

