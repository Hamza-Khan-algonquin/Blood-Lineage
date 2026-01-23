// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnemyLODSkeletalMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API UEnemyLODSkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()
	
	public:
		UEnemyLODSkeletalMeshComponent();

		UFUNCTION(BlueprintCallable)
		void LODUpdate(int newLOD, int oldLOD);
};
