// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ObjectiveLog.generated.h"

UCLASS()
class RELICRUNNERS_API UObjectiveLog : public UUserWidget
{
    GENERATED_BODY()

protected:

    UPROPERTY(meta = (BindWidget))
    class UTreeView* ObjectiveTreeView;

public:

    UFUNCTION(BlueprintCallable, Category = "Objectives")
    void HandleGetItemChildren(UObject* InItem, TArray<UObject*>& OutChildren);

    UFUNCTION(BlueprintCallable)
    void InitializeObjectives(class UObjectiveItemData* RootLevel);
    void AddObjective(UObjectiveItemData* Parent, UObjectiveItemData* NewChild);
    void UpdateObjective(UObjectiveItemData* ObjectiveToUpdate);

};
