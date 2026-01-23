// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "EnemyLODStaticMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API UEnemyLODStaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UEnemyLODStaticMeshComponent();

	UFUNCTION(BlueprintCallable)
	void LODUpdate(int newLOD, int oldLOD);
};
