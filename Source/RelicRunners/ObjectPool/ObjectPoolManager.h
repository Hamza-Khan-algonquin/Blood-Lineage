// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectPoolManager.generated.h"



/* to do list
* Look into switching this to a UWorldSubsystem at a later date. Doing it his way for now because I know who to do it this way so it will be faster to make.
* add timer functionality to set actors inactive after a certain amount of time
* add two seprate arrays, one for active and one for not active 
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
	UFUNCTION(BlueprintCallable)
	void SetActorActive(AActor* actor, bool activeState = true, float lifeSpan = -1);

	UFUNCTION(BlueprintCallable)
	void AddActor( AActor* actorToAdd,  TSubclassOf<AActor> actorType);
	
	//If life span is set to -1 it will stay active indefinitely
	UFUNCTION(BlueprintCallable)
	AActor* GetActor( TSubclassOf<AActor> actorType, float lifeSpan = -1);
	UFUNCTION(BlueprintCallable)
	void RemoveActor(AActor* actorToRemove);
	UFUNCTION(BlueprintCallable)
	void PreLoadPool(TSubclassOf<AActor> actorType, int numberOfElements);

	//neither of these should ever need to be called but their here just incase
	UFUNCTION(BlueprintCallable)
	void CleanUpPool( TSubclassOf<AActor> actorType);
	UFUNCTION(BlueprintCallable)
	void CleanUpPools();
	
	UFUNCTION(BlueprintCallable)
	bool IsActorClassPooled(TSubclassOf<AActor> actorType);

	UFUNCTION(BlueprintCallable, Reliable, NetMulticast)
	void NetMultiCast_OnCreateActor(AActor* newActor);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	AActor* CreateActor(TSubclassOf<AActor> actorType);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
