// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPoolManager.h"
#include "PoolActorInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

TWeakObjectPtr<AObjectPoolManager> AObjectPoolManager::Instance = nullptr;

// Sets default values
AObjectPoolManager::AObjectPoolManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
}

AObjectPoolManager* AObjectPoolManager::GetInstance(const UObject* WorldContext)
{
	UWorld* world = WorldContext->GetWorld();

	if (!world->IsNetMode(NM_DedicatedServer) && !world->IsNetMode(NM_ListenServer))
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolManager::GetInstance called on client! This should only be called on the server!"));
		return nullptr;
	}

	if (Instance.IsValid())
	{
		return Instance.Get();
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(world, AObjectPoolManager::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		Instance = Cast<AObjectPoolManager>(FoundActors[0]);
		return Instance.Get();
	}

	Instance = Cast<AObjectPoolManager>(world->SpawnActor(AObjectPoolManager::StaticClass()));

	return Instance.Get();
}

void AObjectPoolManager::SetActorActive(AActor* actor, bool activeState, float lifeSpan)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolManager SetActorActive called on client! This should only be called on the server!"));
		return;
	}

	TSubclassOf<AActor> actorType = actor->GetClass();

	if (actorType->ImplementsInterface(UPoolActorInterface::StaticClass()))
	{
		IPoolActorInterface* poolActor = Cast<IPoolActorInterface>(actor);
		if (poolActor)
		{
			poolActor->SetIsActive(activeState);
		}
	}
	else
	{
		if (activeState)
		{
			actor->SetActorHiddenInGame(false);
			actor->SetActorEnableCollision(true);
			actor->SetActorTickEnabled(true);
		}
		else
		{
			actor->SetActorHiddenInGame(true);
			actor->SetActorEnableCollision(false);
			actor->SetActorTickEnabled(false);
		}
	}
	
}

void AObjectPoolManager::AddActor(AActor* actorToAdd, TSubclassOf<AActor> actorType)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolManager AddActor called on client! This should only be called on the server!"));
		return;
	}

	PooledObjects.FindOrAdd(actorType).Add(actorToAdd);

	if (!actorType->ImplementsInterface(UPoolActorInterface::StaticClass()) && !PooledObjectsShouldTick.Contains(actorType))
	{
		PooledObjectsShouldTick.Add(actorType, actorToAdd->PrimaryActorTick.bCanEverTick);
	}

	actorToAdd->OnDestroyed.AddDynamic(this, &AObjectPoolManager::RemoveActor);
}

AActor* AObjectPoolManager::CreateActor(TSubclassOf<AActor> actorType)
{
	AActor* newActor = GetWorld()->SpawnActor<AActor>(actorType);

	if (!newActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Object Pool Manager failed to spawn actor of type %s"), *actorType->GetName());
		return nullptr;
	}

	AddActor(newActor, actorType);
	return newActor;
	
}

AActor* AObjectPoolManager::GetActor(TSubclassOf<AActor> actorType, float lifeSpan)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolManager GetActor called on client! This should only be called on the server!"));
		return nullptr;
	}

	if (PooledObjects.Contains(actorType))
	{
		bool isInterface = actorType->ImplementsInterface(UPoolActorInterface::StaticClass());
		for (TWeakObjectPtr<AActor>& pointer : PooledObjects[actorType])
		{
			if (pointer.IsValid() && ((isInterface && !Cast<IPoolActorInterface>(pointer.Get())->GetIsActive()) || (!isInterface && pointer->IsHidden())))
			{
				SetActorActive(pointer.Get(), true, lifeSpan);
				return pointer.Get();
			}
		}
	}

	return CreateActor(actorType);
}

//if this does not work try using actor->IsA(loop through all keys and see if any match)
void AObjectPoolManager::RemoveActor(AActor* actorToRemove)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolManager RemoveActor called on client! This should only be called on the server!"));
		return;
	}

	TSubclassOf<AActor> actorType = actorToRemove->GetClass();

	if (PooledObjects.Contains(actorType))
	{
		PooledObjects[actorType].RemoveAll([actorToRemove](const TWeakObjectPtr<AActor>& pointer)
			{
				return pointer.Get() == actorToRemove;
			});
	}
}

void AObjectPoolManager::PreLoadPool(TSubclassOf<AActor> actorType, int numberOfElements)
{
	PooledObjects.FindOrAdd(actorType).Reserve(numberOfElements);

	for (int i = 0; i < numberOfElements; i++)
	{
		AActor* newActor = CreateActor(actorType);

		if (newActor)
		{
			SetActorActive(newActor, false);
		}
	}
}

void AObjectPoolManager::CleanUpPool(TSubclassOf<AActor> actorType)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolManager RemoveActor called on client! This should only be called on the server!"));
		return;
	}

	if (!PooledObjects.Contains(actorType))
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectPoolManager CleanUpPool called on non existent pool of type %s"), *actorType->GetName());
		return;
	}

	PooledObjects[actorType].RemoveAll([](const TWeakObjectPtr<AActor>& pointer)
		{
			return !pointer.IsValid();
		});
}

void AObjectPoolManager::CleanUpPools()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolManager CleanUpPools called on client! This should only be called on the server!"));
		return;
	}

	for (TMap<TSubclassOf<AActor>, TArray<TWeakObjectPtr<AActor>>>::TIterator it = PooledObjects.CreateIterator(); it; ++it)
	{
		CleanUpPool(it.Key());
	}
}

bool AObjectPoolManager::IsActorClassPooled(TSubclassOf<AActor> actorType)
{
	return PooledObjects.Contains(actorType);
}

// Called when the game starts or when spawned
void AObjectPoolManager::BeginPlay()
{
	if (!HasAuthority())
	{
		return;
	}

	Super::BeginPlay();
	
	if (Instance.IsValid())
	{
		Instance = this;
	}
#if WITH_EDITOR || UE_BUILD_TEST
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Multiple instances of ObjectPoolManager detected!"));
		return;
	}
#endif
}

// Called every frame
void AObjectPoolManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

