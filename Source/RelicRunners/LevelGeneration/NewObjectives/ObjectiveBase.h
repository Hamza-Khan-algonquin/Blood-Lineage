// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectiveBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnObjectiveCreated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnObjectiveActivated);

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

	void SetVisibility(bool newVisibility);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default Values")
	class USceneComponent* Origin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* BeaconMesh;

	FOnObjectiveCreated OnObjectiveCreated;
	FOnObjectiveActivated OnObjectiveActivated;

};
