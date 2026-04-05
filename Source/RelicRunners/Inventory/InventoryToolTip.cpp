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

    TB_ItemTypeTooltip->SetText(FText::FromString(HoveredItem->GetItemType()));
    FLinearColor BorderColor = FLinearColor(0.165f, 0.13f, 0.082f, 1.0f);
    FLinearColor GoldBorderColor = FLinearColor(1, 1, 0.301f, 1.0f);

    TB_LevelTooltip->SetText(FText::FromString(FString::FromInt(HoveredItem->GetLevel())));

    const bool HasEquipped = EquippedItem != nullptr;

    struct FStatInfo
    {
        float Value;
        FString Label;
        UTextBlock* HoveredTextValue;
        UTextBlock* HoveredText;
        UHorizontalBox* HoveredBox;
        UBorder* HoveredBorder;
        UTextBlock* ComparisonTextValue;
    };

    TArray<FStatInfo> Stats = {
        { (float)HoveredItem->ItemData.Health,       "HP",     TB_HealthValue,       TB_HealthTooltip,       HB_Health,       B_Health,       TB_HealthValue_2 },
        { (float)HoveredItem->ItemData.Armor,        "DEF",    TB_ArmorValue,        TB_ArmorTooltip,        HB_Armor,        B_Armor,        TB_ArmorValue_2 },
        { (float)HoveredItem->ItemData.Dexterity,    "AGI",    TB_DexterityValue,    TB_DexterityTooltip,    HB_Dexterity,    B_Dexterity,    TB_DexterityValue_2 },
        { (float)HoveredItem->ItemData.Strength,     "STR",    TB_StrengthValue,     TB_StrengthTooltip,     HB_Strength,     B_Strength,     TB_StrengthValue_2 },
        { (float)HoveredItem->ItemData.Intelligence, "INT",    TB_IntelligenceValue, TB_IntelligenceTooltip, HB_Intelligence, B_Intelligence, TB_IntelligenceValue_2 },
        { (float)HoveredItem->ItemData.Luck,         "LUCK",    TB_LuckValue,         TB_LuckTooltip,         HB_Luck,         B_Luck,         TB_LuckValue_2 },
        { (float)HoveredItem->ItemData.Regen,        "REGEN",    TB_RegenValue,        TB_RegenTooltip,        HB_Regen,        B_Regen,        TB_RegenValue_2 },
        { (float)HoveredItem->ItemData.Purity,       "PURITY",    TB_PurityValue,       TB_PurityTooltip,       HB_Purity,       B_Purity,       TB_PurityValue_2 },
        { (float)HoveredItem->ItemData.Gold,         "Gold",   nullptr,              TB_GoldTooltip,         HB_Gold,         B_Gold,         nullptr }
    };

    for (const FStatInfo& Stat : Stats)
    {
        bool IsSpecialStat = (Stat.Label == "LUCK" || Stat.Label == "REGEN" || Stat.Label == "PURITY");

        if (FMath::IsNearlyZero(Stat.Value) && Stat.Label != "Gold")
        {
            Stat.HoveredBox->SetVisibility(ESlateVisibility::Collapsed);
            Stat.HoveredBorder->SetVisibility(ESlateVisibility::Collapsed);
        }
        else
        {
            if (Stat.HoveredTextValue != nullptr)
            {
                FNumberFormattingOptions Opts;
                Opts.MinimumFractionalDigits = 0; 
                Opts.MaximumFractionalDigits = IsSpecialStat ? 1 : 0;

                FText FormattedValue = FText::AsNumber(Stat.Value, &Opts);
                Stat.HoveredTextValue->SetText(FormattedValue);
            }

            Stat.HoveredBox->SetVisibility(ESlateVisibility::Visible);
            Stat.HoveredBorder->SetVisibility(ESlateVisibility::Visible);
            Stat.HoveredText->SetText(FText::FromString(Stat.Label));

            if (Stat.Label == "Gold")
            {
                Stat.HoveredText->SetText(FText::FromString(FString::FromInt(Stat.Value)));

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
            float EquippedValue = 0.0f;
            if (Stat.Label == "HP") EquippedValue = EquippedItem->ItemData.Health;
            else if (Stat.Label == "DEF") EquippedValue = EquippedItem->ItemData.Armor;
            else if (Stat.Label == "AGI") EquippedValue = EquippedItem->ItemData.Dexterity;
            else if (Stat.Label == "STR") EquippedValue = EquippedItem->ItemData.Strength;
            else if (Stat.Label == "INT") EquippedValue = EquippedItem->ItemData.Intelligence;
            else if (Stat.Label == "LUCK") EquippedValue = EquippedItem->ItemData.Luck;
            else if (Stat.Label == "REGEN") EquippedValue = EquippedItem->ItemData.Regen;
            else if (Stat.Label == "PURITY") EquippedValue = EquippedItem->ItemData.Purity;
            else if (Stat.Label == "Gold") EquippedValue = EquippedItem->ItemData.Gold;

            const float Diff = Stat.Value - EquippedValue;

            if (!FMath::IsNearlyZero(Diff))
            {
                const bool bPositive = Diff > 0;
                FLinearColor PositiveColor = FLinearColor(0, 0.2, 0, 1);
                FLinearColor NegativeColor = FLinearColor(0.2, 0, 0, 1);

                if (Stat.ComparisonTextValue != nullptr)
                {
                    FNumberFormattingOptions DiffOpts;
                    DiffOpts.MinimumFractionalDigits = 0;
                    DiffOpts.MaximumFractionalDigits = IsSpecialStat ? 1 : 0;

                    FText DiffText = FText::AsNumber(FMath::Abs(Diff), &DiffOpts);
                    FString Sign = (Diff > 0) ? "+" : "-";

                    Stat.ComparisonTextValue->SetText(FText::FromString("(" + Sign + DiffText.ToString() + ")"));

                    FLinearColor DiffColor = (Diff > 0) ? FLinearColor(0, 0.2, 0, 1) : FLinearColor(0.2, 0, 0, 1);
                    Stat.ComparisonTextValue->SetColorAndOpacity(DiffColor);
                }
            }
            else if (FMath::IsNearlyEqual(Stat.Value, EquippedValue) && EquippedValue > 0)
            {
                if (Stat.ComparisonTextValue != nullptr)
                {
                    FLinearColor NeutralColor = FLinearColor(0, 0.1, 0.45, 1);
                    Stat.ComparisonTextValue->SetText(FText::FromString("(-)"));
                    Stat.ComparisonTextValue->SetColorAndOpacity(NeutralColor);
                }
            }
        }
    }
}
