// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "ObjectiveItemEntry.generated.h"

UCLASS()
class RELICRUNNERS_API UObjectiveItemEntry : public UUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()
protected:

    UPROPERTY(meta = (BindWidget))
    class UWidgetSwitcher* WidgetSwitcher;

    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
};
