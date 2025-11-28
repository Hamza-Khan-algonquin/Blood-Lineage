// Fill out your copyright notice in the Description page of Project Settings.

#include "PoolActorInterface.h"

void IPoolActorInterface::SetIsActive(bool newIsActive)
{
	AActor* actor = Cast<AActor>(this);
	if (actor)
	{
		if (newIsActive)
		{
			actor->SetActorHiddenInGame(false);
			actor->SetActorEnableCollision(true);
			actor->SetActorTickEnabled(GetShouldTick());
		}
		else
		{
			actor->SetActorHiddenInGame(true);
			actor->SetActorEnableCollision(false);
			actor->SetActorTickEnabled(false);
		}
	}
}
