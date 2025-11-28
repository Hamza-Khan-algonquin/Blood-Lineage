// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PoolActorInterface.generated.h"

/**
 * 
 */
UINTERFACE(MinimalAPI)
class UPoolActorInterface : public UInterface
{
	GENERATED_BODY()
	
};

class RELICRUNNERS_API IPoolActorInterface
{
	GENERATED_BODY()
public:
	virtual bool GetIsActive() = 0;
	virtual bool GetShouldTick() = 0;
	virtual void SetIsActive(bool newIsActive);
};
