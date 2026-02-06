// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectiveBase.h"
#include "NewPayloadObjective.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API ANewPayloadObjective : public AObjectiveBase
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ANewPayloadObjective();

	UPROPERTY()
	class USceneComponent* PayloadOrigin;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* PayloadBody;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* PayloadBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* TriggerCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USplineComponent* SplinePath;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void Initialize(float maxSpeed, float acceleration, float deceleration, class USplineComponent* splinePath);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Initialize(float newSpeed);

	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnTriggerChange(AActor* OtherActor, bool OnOverlapBegin);

	UFUNCTION()
	void OnRep_HasReachedDestination();

	UPROPERTY(Replicated)
	float Distance;

	UPROPERTY(Replicated)
	float Acceleration;

	UPROPERTY(Replicated)
	float Deceleration;

	UPROPERTY(Replicated)
	float MaxSpeed;

	UPROPERTY(Replicated)
	float TargetSpeed;

	UPROPERTY(Replicated)
	float CurrentSpeed;

	UPROPERTY(Replicated)
	bool IsActive;

	UPROPERTY(ReplicatedUsing = OnRep_HasReachedDestination)
	bool HasReachedDestination;

	UPROPERTY(Replicated)
	TArray<AActor*> CollidingActors;
};
