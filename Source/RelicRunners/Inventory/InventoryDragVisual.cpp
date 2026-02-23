// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryDragVisual.h"
#include "RelicRunners/Item/ItemData.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include <RelicRunners/Item/ItemStats.h>

void UInventoryDragVisual::OnSetupVisual(UItemObject* Item)
{
    ItemObject = Item;
    TB_Level->SetText(FText::FromString(FString::FromInt(Item->GetLevel())));

    FLinearColor ItemFrameColor = ItemStats::GetRarityDataMap()[Item->GetRarity()].Color;
    B_Item->SetBrushColor(ItemFrameColor);
    B_Color->SetBrushColor(ItemFrameColor);
    B_Level->SetBrushColor(FLinearColor(0.8, 0.8, 0.6, 1));
    ItemFrame->SetBrushTintColor(ItemFrameColor);
    I_Item->SetBrushFromTexture(Item->ItemData.Icon);

    TB_GoldPrice->SetText(FText::FromString(FString::FromInt(Item->ItemData.Gold)));

    if (Item->IsNewItem) B_NewItem->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    else B_NewItem->SetVisibility(ESlateVisibility::Collapsed);
}
