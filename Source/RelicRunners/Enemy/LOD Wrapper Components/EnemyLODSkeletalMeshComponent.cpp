// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyLODSkeletalMeshComponent.h"

UEnemyLODSkeletalMeshComponent::UEnemyLODSkeletalMeshComponent()
{
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetSimulatePhysics(false);
	bUseRefPoseOnInitAnim = true;
	VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	bEnableUpdateRateOptimizations = true;
	PrimaryComponentTick.bCanEverTick = true;
}

void UEnemyLODSkeletalMeshComponent::InitializeComponent()
{
	SetVisibility(false);
	Super::InitializeComponent();
}

void UEnemyLODSkeletalMeshComponent::LODUpdate(int newLOD, int oldLOD)
{
	if (newLOD == 0 && oldLOD != 0)
	{
		Activate();
		SetVisibility(true);
		SetComponentTickEnabled(true);
		SetComponentTickInterval(0);
		bPauseAnims = false;
	}
	else if(newLOD == 1  && oldLOD != 1)
	{
		Activate();
		SetVisibility(true);
		SetComponentTickEnabled(true);
		SetComponentTickInterval(0.5f);
		bPauseAnims = false;
	}
	else if(newLOD == 2 && oldLOD != 2)
	{
		Activate();
		SetVisibility(true);
		SetComponentTickEnabled(true);
		SetComponentTickInterval(0.25f);
		bPauseAnims = false;
	}
	else if ((newLOD == 3 || newLOD == 4) && !(oldLOD == 3 || oldLOD == 4))
	{
		Deactivate();
		SetVisibility(false);
		SetComponentTickEnabled(false);
		bPauseAnims = true;
	}
}
