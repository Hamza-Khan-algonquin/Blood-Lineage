#include "Director.h"
#include "SpawnPoint.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "RelicRunners/ObjectPool/ObjectPoolManager.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacter.h"
#include "RelicRunners/Enemy/Controllers/SoldierController.h"
#include "NavMesh/RecastNavMesh.h"
#include "Detour/DetourNavMeshQuery.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastQueryFilter.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacter.h"
#include <Net/UnrealNetwork.h>


TWeakObjectPtr<ADirector> ADirector::Instance = nullptr;

// Sets default values
ADirector::ADirector() 
{  
	MaxSpawnPointActivisionDistance = 10000;
	SpawnWait = 1;
	PrimaryActorTick.bCanEverTick = true;
	bAlwaysRelevant = true;
	Tags.Add("Director");
	//SetReplicates(true);
	bReplicates = true;
}

// Called when the game starts or when spawned
void ADirector::BeginPlay()
{
	Super::BeginPlay();
	//make sure we are the server
	if (HasAuthority())
	{
		if (!Instance.IsValid())
		{
			Instance = this;
		}

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

ADirector* ADirector::GetInstance(const UObject* WorldContext)
{
	if (!WorldContext)
	{
		return nullptr;
	}

	UWorld* world = WorldContext->GetWorld();

	if (!world)
	{
		return nullptr;
	}

	if (Instance.IsValid())
	{
		return Instance.Get();
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(world, ADirector::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		Instance = Cast<ADirector>(FoundActors[0]);
		return Instance.Get();
	}

	Instance = Cast<ADirector>(world->SpawnActor(ADirector::StaticClass()));

	return Instance.Get();
}

void ADirector::RemoveEnemy(AActor* enemy)
{
	//cast the actor point as a APawn then check to make sure it when through before destroying it
	APawn* enemyPtr = static_cast<APawn*>(enemy);

	if (!enemyPtr)
	{
		return;
	}

	EnemiesInWorld.Remove(enemyPtr);

	AEnemyCharacter* enemyCharacterPtr = Cast<AEnemyCharacter>(enemyPtr);

	if (enemyCharacterPtr)
	{
		EnemyCharactersInWorld.Remove(enemyCharacterPtr);
		enemyCharacterPtr->OnDestroyed.RemoveDynamic(this, &ADirector::RemoveEnemy);
		enemyCharacterPtr->OnDeactivated.RemoveDynamic(this, &ADirector::RemoveEnemy);
	}
}

void ADirector::AddEnemy(AActor* enemy)
{
	//cast the actor point as a APawn then check to make sure it when through before adding it
	
	if (AEnemyCharacter* enemyCharacterPtr = Cast<AEnemyCharacter>(enemy))
	{
		EnemyCharactersInWorld.Add(enemyCharacterPtr);
	}
	else if (APawn* enemyPtr = Cast<APawn>(enemy))
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
		OnPlayerAdd.Broadcast(newPlayer);
	}
}

void ADirector::RemovePlayer(AActor* player)
{
	if (APawn* pawnPlayer = Cast<APawn>(player))
	{
		Players.Remove(pawnPlayer);

		pawnPlayer->OnDestroyed.RemoveDynamic(this, &ADirector::RemovePlayer);
		OnPlayerRemoved.Broadcast(pawnPlayer);
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

		for (int j = 0; j < Players.Num(); j++)
		{
			if (!Players[j])
			{
				Players.RemoveAt(j);
				j--;
				continue;
			}

			//check to see if the player is close enough and the spawn point is not at max capacity
			if (FVector::Dist2D(Players[j]->GetActorLocation(), point->GetActorLocation()) < MaxSpawnPointActivisionDistance && !point->IsAtMaxCapacity())
			{
				point->SpawnEnemies();
			}
		}
	}
}

#if WITH_EDITOR
void ADirector::GenerateFlowField()
{
	GenerateFlowField(DebugFlowTargetLocation, DebugFlowRadius);
}
#endif

void ADirector::GenerateFlowField(FVector targetLocation, float radius)
{
	UNavigationSystemV1* navSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!navSys)
	{
		return;
	}

	ARecastNavMesh* recastNavMesh = Cast<ARecastNavMesh>(navSys->GetDefaultNavDataInstance());
	if (!recastNavMesh)
	{
		return;
	}

	ActiveNavMesh = recastNavMesh;
	FlowField.Empty();


	FNavLocation targetNavLocation;
	ActiveNavMesh->ProjectPoint(targetLocation, targetNavLocation, { 100,100,100 });
	TArray<NavNodeRef> polys;
	TArray<NavNodeRef> parentPolys;
	TArray<float> costs;
	int polyCount;

	ActiveNavMesh->FindPolysAroundCircle(targetNavLocation.Location, targetNavLocation.NodeRef, radius, ActiveNavMesh->GetDefaultQueryFilter(), this, &polys, &parentPolys, &costs, &polyCount);

	for (int i = 0; i < polyCount; i++)
	{
		if (!parentPolys[i] || !polys[i] || FlowField.Contains(polys[i]))
		{
			continue;
		}

		FVector polyCentre;
		FVector parentPolyCentre;
		ActiveNavMesh->GetPolyCenter(polys[i], polyCentre);
		ActiveNavMesh->GetPolyCenter(parentPolys[i], parentPolyCentre);

		FVector direction = (parentPolyCentre - polyCentre).GetSafeNormal();
		FlowField.Add(polys[i], direction);
		DrawDebugDirectionalArrow(GetWorld(), polyCentre, polyCentre + direction * 100, 100, FColor::Red, true, 5.f, 0, 20.f);
	}
}

void ADirector::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ADirector, Players);
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

