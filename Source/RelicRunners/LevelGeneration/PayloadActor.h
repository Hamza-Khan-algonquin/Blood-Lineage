// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PayloadActor.generated.h"

UCLASS()
class RELICRUNNERS_API APayloadActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APayloadActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* Origin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* PayloadBody;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* TriggerCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USplineComponent* DeliverySpline;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Initialize(class USplineComponent* newSpline, float newSpeed);

	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnTriggerChange(AActor* OtherActor, bool OnOverlapBegin);

	UPROPERTY(Replicated)
	float Distance;

	UPROPERTY(Replicated, EditAnywhere, meta = (ClampMin = "0", UIMin = "0", ClampMax = "1000", UIMax = "1000"))
	float Acceleration;

	UPROPERTY(Replicated, EditAnywhere, meta = (ClampMin = "0", UIMin = "0", ClampMax = "1000", UIMax = "1000"))
	float Deceleration;

	UPROPERTY(Replicated)
	float MaxSpeed;

	UPROPERTY(Replicated)
	float TargetSpeed;

	UPROPERTY(Replicated)
	float CurrentSpeed;

	UPROPERTY(Replicated)
	bool IsActive;

	UPROPERTY(Replicated)
	bool HasReachedDestination;

	UPROPERTY(Replicated)
	TArray<AActor*> CollidingActors;
};
