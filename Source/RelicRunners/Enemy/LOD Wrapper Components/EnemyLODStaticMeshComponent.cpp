// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyLODStaticMeshComponent.h"

UEnemyLODStaticMeshComponent::UEnemyLODStaticMeshComponent()
{
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetSimulatePhysics(false);
}

void UEnemyLODStaticMeshComponent::LODUpdate(int newLOD, int oldLOD)
{
	if ((newLOD == 0 || newLOD == 1 || newLOD == 2) && !(oldLOD == 0 || oldLOD == 1 || newLOD == 2))
	{
		Deactivate();
		SetVisibility(false);
	}
	else if (newLOD == 3 && oldLOD != 3)
	{
		Activate();
		SetVisibility(true);
	}
	else if(newLOD == 4 && oldLOD != 4)
	{
		Deactivate();
		SetVisibility(false);
	}
}
