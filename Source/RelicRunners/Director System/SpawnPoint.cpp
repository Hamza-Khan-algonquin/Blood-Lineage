#include "SpawnPoint.h"
#include <Kismet/GameplayStatics.h>
#include "Director.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacter.h"
#include "RelicRunners/ObjectPool/ObjectPoolManager.h"

// Sets default values
ASpawnPoint::ASpawnPoint()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
		SetRootComponent(Mesh);
		MaxNumOfEnemies = 999;
		Radius = 100;
		HeightToCheckFrom = 200;
		DepthToCheckFrom = 100;
		PrimaryActorTick.bCanEverTick = false;
	}
}

bool ASpawnPoint::IsAtMaxCapacity()
{
	return SpawnedEnemies.Num() >= MaxNumOfEnemies;
}

FVector ASpawnPoint::GetRandomPosition()
{
	bool hasFoundHitPoint = false;
	FHitResult hit(ForceInit);
	int numberOfTries = 0;
	FRandomStream rand;
	rand.GenerateNewSeed();

	//loop until we find a suitable location or we have tried 10 times as their could be no suitable spots
	do
	{
		numberOfTries++;
		hasFoundHitPoint = true;

		//getting said random vector and making both 2d and a direction vector
		FVector vec = rand.GetUnitVector();
		vec.Z = 0;
		vec.Normalize();

		//adjusting it for the radius and adding the spawn points location
		vec *= rand.FRandRange(0, Radius);
		vec += GetActorLocation();

		//Setting up the collision query parameters for the raycast
		FCollisionQueryParams traceParams;
		traceParams.AddIgnoredActor(this);
		traceParams.bTraceComplex = true;

		//Do the raycast
		GetWorld()->LineTraceSingleByChannel(hit, vec + FVector(0, 0, HeightToCheckFrom), vec - FVector(0, 0, DepthToCheckFrom), ECC_PhysicsBody, traceParams);

		//DrawDebugLine(GetWorld(), vec + FVector(0, 0, HeightToCheckFrom), vec - FVector(0, 0, m_depthToCheckToSpawnFrom), FColor::Red, false, 1, 0, 1.f);

		//check to make sure their is enough space between the players and all the enemies
		for (APawn* player : Director->GetPlayers())
		{
			if (!player) continue;

			if (FVector::Dist2D(player->GetActorLocation(), hit.ImpactPoint) <= SpawnSeparationDistance)
			{
				hasFoundHitPoint = false;
				break;
			}
		}

		if (numberOfTries == 10 && !hasFoundHitPoint)
		{
			UE_LOG(LogTemp, Warning, TEXT("Spawn Point could not find a suitable location to spawn after 10 tries"));
			return FVector::ZeroVector;
		}

	} while (!hasFoundHitPoint);

	//return the new point
	return hit.ImpactPoint;
}

void ASpawnPoint::SpawnEnemies()
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if ((SpawnedEnemies.Num() == 0 && !FinishedAllWaves()) || CurrentAllowance > 0)
	{
		//if we have used up all our allowance for this wave then reset it and move to the next wave
		if (CurrentAllowance <= 0)
		{
			CurrentAllowance = SpawnAllowance;
			CurrentWaves++;
		}

		AObjectPoolManager* pool = AObjectPoolManager::GetInstance(this);

		if (!pool)
		{
			return;
		}

		//PickableCards copies SpawningCards or DefaultSpawningCards that way we can remove cards from the pool we have to pick from without losing them on the next call
		if (SpawningCards.Num() > 0)
		{
			PickableCards = SpawningCards;
		}
		else
		{
			PickableCards = Director->GetDefaultSpawnCards();
		}

		//loop through until their is no more allowance for spawning enemies
		while (CurrentAllowance > 0)
		{
			if (IsAtMaxCapacity())
			{
				return;
			}

			//set up local variables
			int pickedEnemyIndex = -1;
			bool pickedSuccessful = false;

			//loop until we have pick a enemy to spawn
			while (!pickedSuccessful)
			{
				//If their are not more cards to pick from then leave the method because we can't spawn any more enemies
				if (PickableCards.Num() <= 0)
				{
					CurrentAllowance = 0;
					return;
				}

				float sumOfWeights = 0;

				//add up all the weights of the enemies to find the max weight
				for (int i = 0; i < PickableCards.Num(); i++)
				{
					sumOfWeights += PickableCards[i]->SpawnWeight;
				}

				//pick a random weight 
				float randomValue = FMath::FRandRange(0, sumOfWeights);

				//loop through and check the weight if the random is less then the weight that our enemy if not subtract the weight from random
				for (int i = 0; i < PickableCards.Num(); i++)
				{
					if (randomValue < PickableCards[i]->SpawnWeight)
					{
						//check to make sure it in the budget for spawning
						if (CurrentAllowance >= PickableCards[i]->SpawnCost)
						{
							pickedEnemyIndex = i;
							CurrentAllowance -= PickableCards[i]->SpawnCost;
							pickedSuccessful = true;
							break;
						}
						//if not remove it from the pool
						else
						{
							PickableCards.RemoveAt(i);
							break;
						}

					}

					randomValue -= PickableCards[i]->SpawnWeight;
				}
			}

			if (CurrentAllowance < 0)
			{
				CurrentAllowance = 0;
			}

			for (int i = 0; i < 10; i++)
			{
				//FActorSpawnParameters SpawnParams;
				//SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;



				//APawn* newEnemy = static_cast<APawn*>(GetWorld()->SpawnActor<APawn>(PickableCards[pickedEnemyIndex]->EnemyClass, GetRandomPosition(), FRotator::ZeroRotator, SpawnParams));
				
				APawn* newEnemy = Cast<APawn>(pool->GetActor(PickableCards[pickedEnemyIndex]->EnemyClass));

				FVector origin;
				FVector bounds;
				newEnemy->GetActorBounds(true, origin, bounds);
				 
				//newEnemy->SetActorLocation(GetRandomPosition() + FVector(0,0, bounds.Z * 2));
				//newEnemy->SetActorLocation(GetRandomPosition(), true);
				newEnemy->SetActorLocation(GetRandomPosition() + FVector(0, 0, bounds.Z));

				//spawning the enemy and adding it to the array
				if (newEnemy)
				{
					newEnemy->OnDestroyed.AddDynamic(this, &ASpawnPoint::RemoveEnemy);

					if (AEnemyCharacter* newEnemyCharacter = Cast<AEnemyCharacter>(newEnemy))
					{
						newEnemyCharacter->OnDeactivated.AddDynamic(this, &ASpawnPoint::RemoveEnemy);
					}

					SpawnedEnemies.Add(newEnemy);
					break;
				}
			}
		}
	}
		
	if(FinishedAllWaves())
	{
		//Justin added this
		OnEnemyWavesComplete.Broadcast();
		//End Justin section
	}
}

void ASpawnPoint::RemoveEnemy(AActor* enemy)
{
	//cast the actor point as a enemy then check to make sure it when through before destroying it
	APawn* enemyPtr = static_cast<APawn*>(enemy);

	if (!enemyPtr)
	{
		return;
	}

	SpawnedEnemies.Remove(enemyPtr);

	AEnemyCharacter* enemyCharacterPtr = Cast<AEnemyCharacter>(enemyPtr);

	if (enemyCharacterPtr)
	{
		enemyCharacterPtr->OnDestroyed.RemoveDynamic(this, &ASpawnPoint::RemoveEnemy);
		enemyCharacterPtr->OnDeactivated.RemoveDynamic(this, &ASpawnPoint::RemoveEnemy);
	}
}

bool ASpawnPoint::FinishedAllWaves()
{
	return CurrentWaves >= NumOfWaves && SpawnedEnemies.Num() <= 0;
}

// Called when the game starts or when spawned
void ASpawnPoint::BeginPlay()
{
	Super::BeginPlay();	
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentWaves = 0;

		if (!Director)
		{
			Director = Cast<ADirector>(UGameplayStatics::GetActorOfClass(GetWorld(), ADirector::StaticClass()));

			if (!Director)
			{
				UE_LOG(LogTemp, Error, TEXT("No ADirector found in scene"));
			}
		}

		//sort enemies by their weights for the weight sums algorithm used when picking who to spawn
		SpawningCards.Sort([](const USpawnCard& A, const USpawnCard& B)
			{
				return A.SpawnWeight < B.SpawnWeight;
			});
	}
}