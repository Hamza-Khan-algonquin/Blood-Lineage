// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveLog.h"
#include "Components/TreeView.h"
#include "ObjectiveItemData.h"
#include "ObjectiveItemEntry.h"

void UObjectiveLog::InitializeObjectives(UObjectiveItemData* RootLevel)
{
    if (!ObjectiveTreeView || !RootLevel) return;


    ObjectiveTreeView->ClearListItems();

    //Level Title Item
    ObjectiveTreeView->AddItem(RootLevel);

    //Expand the root so the objectives are visible immediately
    ObjectiveTreeView->SetItemExpansion(RootLevel, true);
}

void UObjectiveLog::AddObjective(UObjectiveItemData* Parent, UObjectiveItemData* NewChild)
{
    if (!IsValid(Parent) || !IsValid(NewChild) || !ObjectiveTreeView) return;

    //Add the new data to the top of the list
    Parent->SubObjectives.Insert(NewChild, 0);

    ObjectiveTreeView->RequestRefresh();

    //Expand the root so the objectives are visible immediately
    ObjectiveTreeView->SetItemExpansion(Parent, true);
}

void UObjectiveLog::UpdateObjective(UObjectiveItemData* ObjectiveToUpdate)
{
    if (!ObjectiveTreeView || !ObjectiveToUpdate) return;

    ObjectiveTreeView->RegenerateAllEntries();
}

void UObjectiveLog::RefreshLog(UObjectiveItemData* Parent)
{
    if (ObjectiveTreeView)
    {
        ObjectiveTreeView->RequestRefresh();
    }
}

void UObjectiveLog::HandleGetItemChildren(UObject* Item, TArray<UObject*>& OutChildren)
{
    UObjectiveItemData* Data = Cast<UObjectiveItemData>(Item);

    if (Data)
    {
        for (UObjectiveItemData* Child : Data->SubObjectives)
        {
            if (Child)
            {
                OutChildren.Add(Child);
            }
        }
    }
}