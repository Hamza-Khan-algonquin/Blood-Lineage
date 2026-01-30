// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectiveBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFlagObjectiveCreated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFlagObjectiveActivated);

UCLASS()
class RELICRUNNERS_API AObjectiveBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AObjectiveBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Activate();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default Values")
	class USceneComponent* Origin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* BeaconMesh;

	FOnFlagObjectiveCreated OnFlagObjectiveCreated;
	FOnFlagObjectiveActivated OnFlagObjectiveActivated;

};
