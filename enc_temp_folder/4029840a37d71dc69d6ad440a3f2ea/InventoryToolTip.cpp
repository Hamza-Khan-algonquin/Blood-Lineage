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
    TB_RarityTooltip->SetColorAndOpacity(FLinearColor::White);

    TB_ItemTypeTooltip->SetText(FText::FromString(HoveredItem->GetItemType()));
    FLinearColor BorderColor = FLinearColor(0.165f, 0.13f, 0.082f, 1.0f);
    FLinearColor GoldBorderColor = FLinearColor(1, 1, 0.301f, 1.0f);
    //B_ItemType->SetBrushColor(BorderColor);

    TB_LevelTooltip->SetText(FText::FromString(FString::FromInt(HoveredItem->GetLevel())));
    TB_LevelTooltip->SetColorAndOpacity(FLinearColor::White);

    const bool HasEquipped = EquippedItem != nullptr;

    if (HasEquipped)
    {
        FLinearColor EquippedColor = ItemStats::GetRarityDataMap()[EquippedItem->GetRarity()].Color;

        B_Rarity_1->SetBrushColor(EquippedColor);
        TB_RarityTooltip_1->SetText(FText::FromString(EquippedItem->GetRarity()));
        TB_RarityTooltip_1->SetColorAndOpacity(FLinearColor::White);

        TB_ItemTypeTooltip_1->SetText(FText::FromString(EquippedItem->GetItemType()));
        //B_ItemType_1->SetBrushColor(BorderColor);
        TB_LevelTooltip_1->SetText(FText::FromString(FString::FromInt(EquippedItem->GetLevel())));
        TB_LevelTooltip_1->SetColorAndOpacity(FLinearColor::White);
    }
    else
    {
        B_Rarity_1->SetBrushColor(FLinearColor::Transparent);
        TB_RarityTooltip_1->SetText(FText::GetEmpty());

        TB_ItemTypeTooltip_1->SetText(FText::GetEmpty());
        B_ItemType_1->SetBrushColor(FLinearColor::Transparent);
        TB_LevelTooltip_1->SetText(FText::GetEmpty());
    }

    struct FStatInfo
    {
        int Value;
        FString Label;
        UTextBlock* HoveredTextValue;
        UTextBlock* HoveredText;
        UHorizontalBox* HoveredBox;
        UBorder* HoveredBorder;
        UTextBlock* EquippedText;
        UTextBlock* EquippedTextValue;
        UHorizontalBox* EquippedBox;
        UBorder* EquippedBorder;
        UTextBlock* ComparisonTextValue;
        UBorder* ComparisonBorder;
    };

    TArray<FStatInfo> Stats = {
        { HoveredItem->ItemData.Health,       "HP",   TB_HealthValue,      TB_HealthTooltip,       HB_Health,       B_Health,       TB_HealthTooltip_1,       TB_HealthValue_1,         HB_Health_1,       B_Health_1,               TB_HealthValue_2,      B_Health_2},
        { HoveredItem->ItemData.Armor,        "DEF",  TB_ArmorValue,       TB_ArmorTooltip,        HB_Armor,        B_Armor,        TB_ArmorTooltip_1,        TB_ArmorValue_1,          HB_Armor_1,        B_Armor_1,                TB_ArmorValue_2,       B_Armor_2},
        { HoveredItem->ItemData.Dexterity,    "AGI",  TB_DexterityValue,   TB_DexterityTooltip,    HB_Dexterity,    B_Dexterity,    TB_DexterityTooltip_1,    TB_DexterityValue_1,      HB_Dexterity_1,    B_Dexterity_1,        TB_DexterityValue_2,       B_Dexterity_2},
        { HoveredItem->ItemData.Strength,     "STR",  TB_StrengthValue,    TB_StrengthTooltip,     HB_Strength,     B_Strength,     TB_StrengthTooltip_1,     TB_StrengthValue_1,       HB_Strength_1,     B_Strength_1,          TB_StrengthValue_2,       B_Strength_2},
        { HoveredItem->ItemData.Intelligence, "INT",  TB_IntelligenceValue,TB_IntelligenceTooltip, HB_Intelligence, B_Intelligence, TB_IntelligenceTooltip_1, TB_IntelligenceValue_1,   HB_Intelligence_1, B_Intelligence_1,  TB_IntelligenceValue_2,       B_Intelligence_2},
        { HoveredItem->ItemData.Gold,         "Gold", nullptr,             TB_GoldTooltip,         HB_Gold,         B_Gold,         TB_GoldTooltip_1,         nullptr,                  HB_Gold_1,         B_Gold_1,                         nullptr,       nullptr}
    };

    B_Equipped->SetVisibility(HasEquipped ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

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
            Stat.HoveredText->SetColorAndOpacity(FLinearColor::White);
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

            // Equipped display
            if (EquippedValue == 0 && Stat.Label != "Gold")
            {
                Stat.EquippedBox->SetVisibility(ESlateVisibility::Collapsed);
                Stat.EquippedBorder->SetVisibility(ESlateVisibility::Collapsed);
            }
            else
            {
                Stat.EquippedBox->SetVisibility(ESlateVisibility::Visible);
                Stat.EquippedBorder->SetVisibility(ESlateVisibility::Visible);

                // Display equipped stat with label, "HP 5"
                if (Stat.EquippedTextValue != nullptr)
                {
                    Stat.EquippedTextValue->SetText(FText::FromString(FString::FromInt(EquippedValue)));
                }
                Stat.EquippedText->SetText(FText::FromString(Stat.Label));
                Stat.EquippedText->SetColorAndOpacity(FLinearColor::White);
                //Stat.EquippedBorder->SetBrushColor(BorderColor);

                if (Stat.Label == "Gold")
                {
                    Stat.EquippedText->SetText(FText::FromString(FString::FromInt(EquippedValue)));
                    Stat.EquippedText->SetColorAndOpacity(FLinearColor(1,1,0.3,1));
                    //Stat.EquippedBorder->SetBrushColor(GoldBorderColor);
                }
            }

            // Comparison display
            B_Comparison->SetVisibility(ESlateVisibility::Visible);
            if(Stat.ComparisonBorder) Stat.ComparisonBorder->SetVisibility(ESlateVisibility::Visible);
            if (Diff != 0)
            {
                const bool bPositive = Diff > 0;
                FLinearColor DiffColor = bPositive ? FLinearColor::Green : FLinearColor::Red;
                if (Stat.ComparisonTextValue != nullptr)
                {
                    Stat.ComparisonTextValue->SetText(FText::FromString((bPositive ? "+" : "") + FString::FromInt(Diff)));
                    Stat.ComparisonTextValue->SetColorAndOpacity(DiffColor);
                }
            }
            else if (Diff == 0 && EquippedValue > 0 && Stat.Value > 0)
            {
                
                if (Stat.ComparisonTextValue != nullptr)
                {
                    Stat.ComparisonTextValue->SetText(FText::FromString("-"));
                    Stat.ComparisonTextValue->SetColorAndOpacity(FLinearColor(1,0.5f,0,1));
                }
            }
            else
            {
                Stat.ComparisonBorder->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
        else
        {
            //No equipped item: collapse equipped
            Stat.EquippedBox->SetVisibility(ESlateVisibility::Collapsed);
            B_Comparison->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}
