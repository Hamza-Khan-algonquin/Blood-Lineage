// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CullingToggleInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCullingToggleInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RELICRUNNERS_API ICullingToggleInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintImplementableEvent)
	void ToggleVisibility(bool newVisibility);
};
