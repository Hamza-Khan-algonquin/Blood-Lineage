/************************************************************************************
 * Author:   Tristan Anglin
 * Year:     2025
 *
 * Notice:
 *   This source code is the intellectual property of Tristan Anglin.
 *   It is provided solely for use within:
 *     - The Relic Runners personal project
 *     - The Almost A Dozen project
 *
 *   Any use, distribution, or modification outside of these projects
 *   is strictly prohibited without explicit written permission.
 *
 *   © 2025 Tristan Anglin. All rights reserved.
 ************************************************************************************/

#include "InventoryToolTip.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CanvasPanelSlot.h"
#include "RelicRunners/Item/ItemData.h"
#include "InventoryComponent.h"
#include <RelicRunners/Item/ItemStats.h>

void UInventoryToolTip::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

void UInventoryToolTip::Setup(FItemData& ItemData, const FLinearColor& RarityColor)
{
    UItemObject* Item = NewObject<UItemObject>(this);
    Item->SetItemData(ItemData);

    Setup(Item, RarityColor, nullptr);
}

void UInventoryToolTip::Setup(UItemObject* HoveredItem, const FLinearColor& HoveredColor, UItemObject* EquippedItem)
{
    B_Rarity->SetBrushColor(HoveredColor);
    TB_RarityTooltip->SetText(FText::FromString(HoveredItem->GetRarity()));
    //TB_RarityTooltip->SetColorAndOpacity(FLinearColor::White);

    TB_ItemTypeTooltip->SetText(FText::FromString(HoveredItem->GetItemType()));
    FLinearColor BorderColor = FLinearColor(0.165f, 0.13f, 0.082f, 1.0f);
    FLinearColor GoldBorderColor = FLinearColor(1, 1, 0.301f, 1.0f);
    //B_ItemType->SetBrushColor(BorderColor);

    TB_LevelTooltip->SetText(FText::FromString(FString::FromInt(HoveredItem->GetLevel())));
    //TB_LevelTooltip->SetColorAndOpacity(FLinearColor::White);

    const bool HasEquipped = EquippedItem != nullptr;

    struct FStatInfo
    {
        int Value;
        FString Label;
        UTextBlock* HoveredTextValue;
        UTextBlock* HoveredText;
        UHorizontalBox* HoveredBox;
        UBorder* HoveredBorder;
        UTextBlock* ComparisonTextValue;
    };

    TArray<FStatInfo> Stats = {
        { HoveredItem->ItemData.Health,       "HP",   TB_HealthValue,      TB_HealthTooltip,       HB_Health,       B_Health,       TB_HealthValue_2 },
        { HoveredItem->ItemData.Armor,        "DEF",  TB_ArmorValue,       TB_ArmorTooltip,        HB_Armor,        B_Armor,        TB_ArmorValue_2 },
        { HoveredItem->ItemData.Dexterity,    "AGI",  TB_DexterityValue,   TB_DexterityTooltip,    HB_Dexterity,    B_Dexterity,    TB_DexterityValue_2 },
        { HoveredItem->ItemData.Strength,     "STR",  TB_StrengthValue,    TB_StrengthTooltip,     HB_Strength,     B_Strength,     TB_StrengthValue_2 },
        { HoveredItem->ItemData.Intelligence, "INT",  TB_IntelligenceValue,TB_IntelligenceTooltip, HB_Intelligence, B_Intelligence, TB_IntelligenceValue_2 },
        { HoveredItem->ItemData.Gold,         "Gold", nullptr,             TB_GoldTooltip,         HB_Gold,         B_Gold,         nullptr }
    };

    for (const FStatInfo& Stat : Stats)
    {
        if (Stat.Value == 0 && Stat.Label != "Gold" )
        {
            Stat.HoveredBox->SetVisibility(ESlateVisibility::Collapsed);
            Stat.HoveredBorder->SetVisibility(ESlateVisibility::Collapsed);
        }
        else
        {
            if (Stat.HoveredTextValue != nullptr)
            {
                Stat.HoveredTextValue->SetText(FText::FromString(FString::FromInt(Stat.Value)));
            }

            Stat.HoveredBox->SetVisibility(ESlateVisibility::Visible);
            Stat.HoveredBorder->SetVisibility(ESlateVisibility::Visible);
            Stat.HoveredText->SetText(FText::FromString(Stat.Label));
            //Stat.HoveredText->SetColorAndOpacity(FLinearColor::White);
            //Stat.HoveredBorder->SetBrushColor(BorderColor);

            if (Stat.Label == "Gold")
            {
                Stat.HoveredText->SetText(FText::FromString(FString::FromInt(Stat.Value)));
                Stat.HoveredText->SetColorAndOpacity(FLinearColor(1, 1, 0.3, 1));
                //Stat.HoveredBorder->SetBrushColor(GoldBorderColor);

                if (HoveredItem->ItemData.ItemType == "Greatsword"
                    || HoveredItem->ItemData.ItemType == "Sword"
                    || HoveredItem->ItemData.ItemType == "Staff"
                    || HoveredItem->ItemData.ItemType == "Bow")
                {
                    Stat.HoveredBox->SetVisibility(ESlateVisibility::Collapsed);
                    Stat.HoveredBorder->SetVisibility(ESlateVisibility::Collapsed);
                }
            }
        }

        if (HasEquipped)
        {
            int EquippedValue = 0;
            if (Stat.Label == "HP") EquippedValue = EquippedItem->ItemData.Health;
            else if (Stat.Label == "DEF") EquippedValue = EquippedItem->ItemData.Armor;
            else if (Stat.Label == "AGI") EquippedValue = EquippedItem->ItemData.Dexterity;
            else if (Stat.Label == "STR") EquippedValue = EquippedItem->ItemData.Strength;
            else if (Stat.Label == "INT") EquippedValue = EquippedItem->ItemData.Intelligence;
            else if (Stat.Label == "Gold") EquippedValue = EquippedItem->ItemData.Gold;

            const int Diff = Stat.Value - EquippedValue;

            if (Diff != 0)
            {
                const bool bPositive = Diff > 0;
                FLinearColor DiffColor = bPositive ? FLinearColor::Green : FLinearColor::Red;
                if (Stat.ComparisonTextValue != nullptr)
                {
                    Stat.ComparisonTextValue->SetText(FText::FromString((bPositive ? "[+" : "[") + FString::FromInt(Diff) + ("]")));
                    Stat.ComparisonTextValue->SetColorAndOpacity(DiffColor);

                    Stat.HoveredTextValue->SetText(FText::FromString(FString::FromInt(Stat.Value)));
                    Stat.HoveredBox->SetVisibility(ESlateVisibility::Visible);
                    Stat.HoveredBorder->SetVisibility(ESlateVisibility::Visible);
                }
            }
            else if (Diff == 0 && EquippedValue > 0 && Stat.Value > 0)
            {
                
                if (Stat.ComparisonTextValue != nullptr)
                {
                    Stat.ComparisonTextValue->SetText(FText::FromString("[-]"));
                    Stat.ComparisonTextValue->SetColorAndOpacity(FLinearColor(1,0.5f,0,1));
                }
            }
        }
    }
}
