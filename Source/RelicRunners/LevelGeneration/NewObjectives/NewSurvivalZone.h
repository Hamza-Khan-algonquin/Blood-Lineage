// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/TimelineComponent.h"
#include "CoreMinimal.h"
#include "ObjectiveBase.h"
#include "NewSurvivalZone.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API ANewSurvivalZone : public AObjectiveBase
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ANewSurvivalZone();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* TriggerZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UTimelineComponent* SurvivalTimelineComp;

	UPROPERTY(EditAnywhere)
	UCurveFloat* SurvivalTimelineFloatCurve;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default Values")
	USkeletalMeshComponent* PandoraBoxMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Values")
	UAnimSequence* ActivateAnimation;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void UpdateTimelineComp(float Output);

	UFUNCTION()
	void OnTimelineEnd();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HandleOverlapEnter(AActor* OtherActor, bool OnOverlapBegin);

	UPROPERTY(Replicated, EditAnywhere)
	float Duration;

	UPROPERTY(Replicated)
	float Remaining;

	UPROPERTY(Replicated)
	TArray<AActor*> CollidingActors;
};
