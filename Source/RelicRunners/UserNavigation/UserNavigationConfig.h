// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Application/NavigationConfig.h"
#include "Framework/Application/SlateApplication.h"

class FUserNavigationConfig : public FNavigationConfig
{
public:
    FUserNavigationConfig()
    {
        KeyEventRules.Empty();

        bTabNavigation = false;
        bKeyNavigation = false;
        bAnalogNavigation = false;
    }
};

