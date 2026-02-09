// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveLog.h"
#include "Components/TreeView.h"
#include "ObjectiveItemData.h"

void UObjectiveLog::InitializeObjectives(UObjectiveItemData* RootLevel)
{
    if (!ObjectiveTreeView || !RootLevel) return;

    //Clear previous entries
    ObjectiveTreeView->ClearListItems();

    //Level Title Item
    ObjectiveTreeView->AddItem(RootLevel);

    //Expand the root so the objectives are visible immediately
    ObjectiveTreeView->SetItemExpansion(RootLevel, true);
}

void UObjectiveLog::AddObjective(UObjectiveItemData* Parent, UObjectiveItemData* NewChild)
{
    if (!Parent || !NewChild || !ObjectiveTreeView) return;

    //Add the new data to the parent's array
    Parent->SubObjectives.Insert(NewChild, 0);

    //Trigger HandleGetItemChildren to run again for this item
    ObjectiveTreeView->RequestRefresh();

    //Expand the root so the objectives are visible immediately
    ObjectiveTreeView->SetItemExpansion(Parent, true);
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