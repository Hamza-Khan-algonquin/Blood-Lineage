// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShockwaveRing.generated.h"

UCLASS()
class RELICRUNNERS_API AShockwaveRing : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShockwaveRing();

	void InitializeRing(int Difficulty, float Speed);

	UFUNCTION()
	void OnSliceOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UPROPERTY(EditAnywhere, Category = "Shockwave")
	UStaticMesh* SliceMesh;
	float MoveSpeed;
	float CurrentScale = 1.0f;
	float MaxRadius = 8000.0f;
	float Acceleration = 0.15f;
	int Difficulty = 1;
	TMap<USceneComponent*, FVector> SliceDirections;
protected:

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
