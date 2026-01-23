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
	if ((newLOD == 0 || newLOD == 1) && !(oldLOD == 0 || oldLOD == 1))
	{
		Deactivate();
		SetVisibility(false);
	}
	else if ((newLOD == 2 || newLOD == 3) && !(oldLOD == 2 || oldLOD == 3))
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
