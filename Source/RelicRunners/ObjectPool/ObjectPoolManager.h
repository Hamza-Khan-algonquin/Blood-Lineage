// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectPoolManager.generated.h"



/* to do list
* Look into switching this to a UWorldSubsystem at a later date. Doing it his way for now because I know who to do it this way so it will be faster to make.
* add timer functionality to set actors inactive after a certain amount of time
* add a method to preload certain amount of actors into the pool 
*/ 
UCLASS()
class RELICRUNNERS_API AObjectPoolManager : public AActor
{
	GENERATED_BODY()
protected:
	//using TWeakObjectPtr as it will automatically become nullptr if the object is destroyed and is not owned by the actor itself
	//YEA BITCH!!! SINGLETON OOHH!!!!  
	static TWeakObjectPtr<AObjectPoolManager> Instance;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object Pool Manager")
	TMap<TSubclassOf<AActor>, TArray<TWeakObjectPtr<AActor>>> PooledObjects;
	TMap<TSubclassOf<AActor>, bool> PooledObjectsShouldTick;

public:	
	// Sets default values for this actor's properties
	AObjectPoolManager();
	
	static AObjectPoolManager* GetInstance(const UObject* WorldContext);

	//If life span is set to -1 it will stay active indefinitely
	void SetActorActive(AActor* actor, bool activeState = true, float lifeSpan = -1);

	void AddActor( AActor* actorToAdd,  TSubclassOf<AActor> actorType);
	
	//If life span is set to -1 it will stay active indefinitely
	AActor* GetActor( TSubclassOf<AActor> actorType, float lifeSpan = -1);
	UFUNCTION()
	void RemoveActor(AActor* actorToRemove);

	//neither of these should ever need to be called but their here just incase
	void CleanUpPool( TSubclassOf<AActor> actorType);
	void CleanUpPools();

	bool IsActorClassPooled(TSubclassOf<AActor> actorType);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	AActor* CreateActor(TSubclassOf<AActor> actorType);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
