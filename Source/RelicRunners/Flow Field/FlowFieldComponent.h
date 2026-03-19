// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "FlowFieldComponent.generated.h"

class ARecastNavMesh;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RELICRUNNERS_API UFlowFieldComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	//NavNodeRef is just a uint64 so I use that instead of a FNavNodeRef to avoid a include
	TMap<uint64, FVector> FlowField;
	UPROPERTY()
	uint64 CurrentNodeRef;
	UPROPERTY()
	ARecastNavMesh* ActiveNavMesh;
	UPROPERTY(EditAnywhere)
	float FieldRadius;
protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	bool DrawFlowField = false;
#endif
public:	
	// Sets default values for this component's properties
	UFlowFieldComponent();
	void EndPlay(const EEndPlayReason::Type EndPlayReason);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void GenerateFlowField(FVector targetLocation);
	void GenerateFlowField(FNavLocation targetNavLocation);

	//respodible for generating the flow field for a given target location and radius. It will store the flow field in the FlowField map, where the key is the NavNodeRef of the cell and the value is the direction to move in that cell to get to the target location
	UFUNCTION()
	void GenerateFlowField(FVector targetLocation, float radius);
	void GenerateFlowField(FNavLocation targetNavLocation, float radius);

	UFUNCTION()
	void CheckForInvalidNodeRefs(ANavigationData* navData);
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
