// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyLODSkeletalMeshComponent.h"

UEnemyLODSkeletalMeshComponent::UEnemyLODSkeletalMeshComponent()
{
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetSimulatePhysics(false);
}

void UEnemyLODSkeletalMeshComponent::LODUpdate(int newLOD, int oldLOD)
{
	if ((newLOD == 0 || newLOD == 1) && !(oldLOD == 0 || oldLOD == 1))
	{
		Activate();
		SetVisibility(true);
	}
	else if((newLOD == 2 || newLOD == 3 || newLOD == 4) && !(oldLOD == 2 || oldLOD == 3 || oldLOD == 4))
	{
		Deactivate();
		SetVisibility(false);
	}
}
