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

#pragma once

#include "CoreMinimal.h"
#include "ItemData.h"
#include "ItemMeshData.h"
#include "IconLibrary.h"

struct FTooltipData
{
public:
    FTooltipData() {}

    UPROPERTY()
    FText Name;

    UPROPERTY()
    FText Value;

    UPROPERTY()
    TArray<FText> Descriptions;

    UPROPERTY()
    class UTexture2D* Icon;

    UPROPERTY()
    FLinearColor IconColor;

    UPROPERTY()
    class UTexture2D* Icon1;

    UPROPERTY()
    FLinearColor IconColor1;

    UPROPERTY()
    class UTexture2D* Icon2;

    UPROPERTY()
    FLinearColor IconColor2;

    UPROPERTY()
    class UTexture2D* Icon3;

    UPROPERTY()
    FLinearColor IconColor3;
};

struct FForgeResultOdds
{
public:
    FForgeResultOdds() {}

    UPROPERTY() 
    FString Rarity1;
    UPROPERTY() 
    float Chance1 = 0.f;

    UPROPERTY() 
    FString Rarity2;
    UPROPERTY() 
    float Chance2 = 0.f;

    UPROPERTY() 
    FString Rarity3;
    UPROPERTY() 
    float Chance3 = 0.f;
};

class RELICRUNNERS_API ItemStats
{
public:

    static FTooltipData GetStatTooltipData(FString BorderType, int StatValue, int StatValue2, int StatValue3, const FString& AbilityName, const FString& AbilityDesc)
    {
        FTooltipData Data;

        if (!BorderType.Contains("Ability") && !BorderType.Contains("Difficulty") && !BorderType.Contains("HealthPotion"))
        {
            Data.Value = FText::AsNumber(StatValue);
        }

        if (BorderType == "Health")
        {
            Data.Name = FText::FromString("Health");
            Data.Icon = LoadIcon("HealthIcon");
            Data.IconColor = GetIconColor("Health");
            Data.Descriptions.Add(FText::FromString("<Default>Health pool.</>"));
        }
        if (BorderType == "MetaHealth")
        {
            Data.Name = FText::FromString("Health");
            Data.Icon = LoadIcon("HealthIcon");
            Data.IconColor = GetIconColor("Health");
            Data.Descriptions.Add(FText::FromString("<Default>Health pool.</>"));
            Data.Descriptions.Add(FText::FromString("<Blue>+10</> <Default>Health per meta point.</>"));
        }
        else if (BorderType == "Defence")
        {
            Data.Name = FText::FromString("Defence");
            Data.Icon = LoadIcon("DefenceIcon");
            Data.IconColor = GetIconColor("Defence");
            Data.Descriptions.Add(FText::Format(FText::FromString("<Green>{0}%</> <Default>Damage reduction.</>"), FText::AsNumber(GetDefencePerc(StatValue))));
        }
        else if (BorderType == "MetaDefence")
        {
            Data.Name = FText::FromString("Defence");
            Data.Icon = LoadIcon("DefenceIcon");
            Data.IconColor = GetIconColor("Defence");
            Data.Descriptions.Add(FText::Format(FText::FromString("<Blue>+{0}%</> <Default>Damage reduction.</>"), FText::AsNumber(GetDefencePerc(StatValue))));
            Data.Descriptions.Add(FText::FromString("<Blue>+3</> <Default>Defence per meta point.</>"));
        }
        else if (BorderType == "Strength")
        {
            Data.Name = FText::FromString("Strength");
            Data.Icon = LoadIcon("StrengthIcon");
            Data.IconColor = GetIconColor("Strength");
            Data.Descriptions.Add(FText::Format(FText::FromString("<Green>+{0}%</> <Default>Attack & ability damage.</>"), FText::AsNumber(GetStrengthPerc(StatValue))));
        }
        else if (BorderType == "MetaStrength")
        {
            Data.Name = FText::FromString("Strength");
            Data.Icon = LoadIcon("StrengthIcon");
            Data.IconColor = GetIconColor("Strength");
            Data.Descriptions.Add(FText::Format(FText::FromString("<Blue>+{0}%</> <Default>Attack & ability damage.</>"), FText::AsNumber(GetStrengthPerc(StatValue))));
            Data.Descriptions.Add(FText::FromString("<Blue>+3</> <Default>Strength per meta point.</>"));
        }
        else if (BorderType == "Agility")
        {
            Data.Name = FText::FromString("Agility");
            Data.Icon = LoadIcon("AgilityIcon");
            Data.IconColor = GetIconColor("Agility");
            Data.Descriptions.Add(FText::Format(FText::FromString("<Green>+{0}%</> <Default> Crit chance.</>"), FText::AsNumber(GetCritPerc(StatValue))));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Green>{0}x</> <Default> Damage on crit.</>"), FText::AsNumber(GetCritDmgPerc(StatValue))));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Green>+{0}%</> <Default>Movement speed.</>"), FText::AsNumber(GetSpeedPerc(StatValue))));
        }
        else if (BorderType == "MetaAgility")
        {
            Data.Name = FText::FromString("Agility");
            Data.Icon = LoadIcon("AgilityIcon");
            Data.IconColor = GetIconColor("Agility");
            Data.Descriptions.Add(FText::Format(FText::FromString("<Blue>+{0}%</> <Default>Crit chance.</>"), FText::AsNumber(GetCritPerc(StatValue))));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Blue>{0}x</> <Default>Damage on crit.</>"), FText::AsNumber(GetCritDmgPerc(StatValue))));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Blue>+{0}%</> <Default>Movement speed.</>"), FText::AsNumber(GetSpeedPerc(StatValue))));
            Data.Descriptions.Add(FText::FromString("<Blue>+3</> <Default>Agility per meta point.</>"));
        }
        else if (BorderType == "Intelligence")
        {
            Data.Name = FText::FromString("Intelligence");
            Data.Icon = LoadIcon("IntelligenceIcon");
            Data.IconColor = GetIconColor("Intelligence");
            Data.Descriptions.Add(FText::Format(FText::FromString("<Green>+{0}</> <Default>Mana pool.</>"), FText::AsNumber(GetManaPoolIncrease(StatValue))));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Green>{0}%</> <Default>Ability cost reduction.</>"), FText::AsNumber(GetAbilityCostPerc(StatValue))));
        }
        else if (BorderType == "MetaIntelligence")
        {
            Data.Name = FText::FromString("Intelligence");
            Data.Icon = LoadIcon("IntelligenceIcon");
            Data.IconColor = GetIconColor("Intelligence");
            Data.Descriptions.Add(FText::Format(FText::FromString("<Blue>+{0}</> <Default>Mana pool.</>"), FText::AsNumber(GetManaPoolIncrease(StatValue))));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Blue>+{0}%</> <Default>Ability cost reduction.</>"), FText::AsNumber(GetAbilityCostPerc(StatValue))));
            Data.Descriptions.Add(FText::FromString("<Blue>+3</> <Default>Intelligence per meta point.</>"));
        }
        else if (BorderType == "Luck")
        {
            Data.Name = FText::FromString("Luck");
            Data.Icon = LoadIcon("LuckIcon");
            Data.IconColor = GetIconColor("Luck");
            Data.Descriptions.Add(FText::Format(FText::FromString("<Green>+{0}%</> <Default>Loot chance.</>"), FText::AsNumber(GetLootChance(StatValue))));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Green>+{0}%</> <Default>Loot quality.</>"), FText::AsNumber(GetLootQuality(StatValue))));
        }
        else if (BorderType == "MetaLuck")
        {
            Data.Name = FText::FromString("Luck");
            Data.Icon = LoadIcon("LuckIcon");
            Data.IconColor = GetIconColor("Luck");
            Data.Descriptions.Add(FText::Format(FText::FromString("<Blue>+{0}%</> <Default>Loot chance.</>"), FText::AsNumber(GetLootChance(StatValue))));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Blue>+{0}%</> <Default>Loot quality.</>"), FText::AsNumber(GetLootQuality(StatValue))));
            Data.Descriptions.Add(FText::FromString("<Blue>+1</> <Default>Luck per meta point.</>"));
        }
        else if (BorderType == "HealthPotion")
        {
            Data.Name = FText::FromString("Health Potion");
            Data.Icon = LoadIcon("PotionIcon");
            Data.IconColor = GetIconColor("Purity");
            Data.Descriptions.Add(FText::FromString("<Default>Heals player based off of purity stat.</>"));
        }
        else if (BorderType == "Purity")
        {
            Data.Name = FText::FromString("Purity");
            Data.Icon = LoadIcon("PotionIcon");
            Data.IconColor = GetIconColor("Purity");
            Data.Descriptions.Add(FText::Format(FText::FromString("<Green>{0}%</> <Default>Health potion heal.</>"), FText::AsNumber(GetHealPercent(StatValue))));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Green>{0}%</> <Default>Chance to not consume.</>"), FText::AsNumber(GetChanceToNotConsume(StatValue))));
        }
        else if (BorderType == "MetaPurity")
        {
            Data.Name = FText::FromString("Purity");
            Data.Icon = LoadIcon("PotionIcon");
            Data.IconColor = GetIconColor("Purity");
            Data.Descriptions.Add(FText::Format(FText::FromString("<Blue>+{0}%</> <Default>Health potion heal.</>"), FText::AsNumber(GetHealPercent(StatValue))));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Blue>+{0}%</> <Default>Chance to not consume.</>"), FText::AsNumber(GetChanceToNotConsume(StatValue))));
            Data.Descriptions.Add(FText::FromString("<Blue>+5</> <Default>Purity per meta point.</>"));
        }
        else if (BorderType == "Level")
        {
            Data.Name = FText::FromString("Level");
            Data.Icon = LoadIcon("StartingLevelIcon");
            Data.IconColor = GetIconColor("Level");
            Data.Descriptions.Add(FText::FromString("<Default>Current Level.</>"));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Blue>+{0}</> <Default>From Meta-Upgrade.</>"), FText::AsNumber(StatValue2)));
        }
        else if (BorderType == "MetaLevel")
        {
            Data.Name = FText::FromString("Meta Level");
            Data.Icon = LoadIcon("StartingLevelIcon");
            Data.IconColor = GetIconColor("MetaLevel");
            Data.Descriptions.Add(FText::FromString("<Default>Current Meta Level.</>"));
            Data.Descriptions.Add(FText::FromString("<Blue>+1</> <Default>Meta point per meta level.</>"));
            Data.Descriptions.Add(FText::FromString("<Default>Meta points are spent in the upgrades menu.</>"));
            }
        else if (BorderType == "MetaStartingLevel")
        {
            Data.Name = FText::FromString("Starting Level");
            Data.Icon = LoadIcon("StartingLevelIcon");
            Data.IconColor = GetIconColor("Level");
            Data.Descriptions.Add(FText::FromString("<Blue>+1</> <Default>Starting level per meta point.</>"));
        }
        else if (BorderType == "Mana")
        {
            Data.Name = FText::FromString("Mana");
            Data.Icon = LoadIcon("ResourceIcon");
            Data.IconColor = GetIconColor("Mana");
            Data.Descriptions.Add(FText::FromString("Mana pool."));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Green>+{0}%</> <Default>Mana / {1}s.</>"), FText::AsNumber(StatValue2), FText::AsNumber(StatValue3)));
        }
        else if (BorderType == "MetaMana")
        {
            Data.Name = FText::FromString("Mana Regen");
            Data.Icon = LoadIcon("ResourceIcon");
            Data.IconColor = GetIconColor("Mana");
            Data.Descriptions.Add(FText::Format(FText::FromString("<Blue>+{0}%</> <Default>Mana / {1}s.</>"), FText::AsNumber(StatValue2), FText::AsNumber(StatValue3)));
            Data.Descriptions.Add(FText::FromString("<Blue>+1%</> <Default>Mana regen per meta point.</>"));
        }
        else if (BorderType == "BoarDifficulty")
        {
            Data.Name = FText::FromString("Boar | Difficulty");
            Data.Icon = LoadIcon("BoarDifficulty");
            Data.IconColor = FLinearColor(1,0.5,0,1);
            Data.Icon1 = LoadIcon("EnemyHealth");
            Data.IconColor1 = GetIconColor("EnemyHealth");
            Data.Icon2 = LoadIcon("EnemyDamage");
            Data.IconColor2 = GetIconColor("EnemyDamage");
            Data.Icon3 = LoadIcon("LuckIcon");
            Data.IconColor3 = GetIconColor("Luck");
            Data.Descriptions.Add(FText::Format(FText::FromString("<EnemyHealth>{0}%</> <Default>Enemy Health</>"), FText::AsNumber(StatValue)));
            Data.Descriptions.Add(FText::Format(FText::FromString("<EnemyDamage>{0}%</> <Default>Enemy Damage</>"), FText::AsNumber(StatValue2)));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Luck>+{0}</> <Default>Luck</>"), FText::AsNumber(StatValue3)));
        }
        else if (BorderType == "BullDifficulty")
        {
            Data.Name = FText::FromString("Bull | Difficulty");
            Data.Icon = LoadIcon("BullDifficulty");
            Data.IconColor = FLinearColor(1, 0.5, 0, 1);
            Data.Icon1 = LoadIcon("EnemyHealth");
            Data.IconColor1 = GetIconColor("EnemyHealth");
            Data.Icon2 = LoadIcon("EnemyDamage");
            Data.IconColor2 = GetIconColor("EnemyDamage");
            Data.Icon3 = LoadIcon("LuckIcon");
            Data.IconColor3 = GetIconColor("Luck");
            Data.Descriptions.Add(FText::Format(FText::FromString("<EnemyHealth>{0}%</> <Default>Enemy Health</>"), FText::AsNumber(StatValue)));
            Data.Descriptions.Add(FText::Format(FText::FromString("<EnemyDamage>{0}%</> <Default>Enemy Damage</>"), FText::AsNumber(StatValue2)));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Luck>+{0}</> <Default>Luck</>"), FText::AsNumber(StatValue3)));
        }
        else if (BorderType == "LionDifficulty")
        {
            Data.Name = FText::FromString("Lion | Difficulty");
            Data.Icon = LoadIcon("LionDifficulty");
            Data.IconColor = FLinearColor(1, 0.5, 0, 1);
            Data.Icon1 = LoadIcon("EnemyHealth");
            Data.IconColor1 = GetIconColor("EnemyHealth");
            Data.Icon2 = LoadIcon("EnemyDamage");
            Data.IconColor2 = GetIconColor("EnemyDamage");
            Data.Icon3 = LoadIcon("LuckIcon");
            Data.IconColor3 = GetIconColor("Luck");
            Data.Descriptions.Add(FText::Format(FText::FromString("<EnemyHealth>{0}%</> <Default>Enemy Health</>"), FText::AsNumber(StatValue)));
            Data.Descriptions.Add(FText::Format(FText::FromString("<EnemyDamage>{0}%</> <Default>Enemy Damage</>"), FText::AsNumber(StatValue2)));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Luck>+{0}</> <Default>Luck</>"), FText::AsNumber(StatValue3)));
        }
        else if (BorderType == "HydraDifficulty")
        {
            Data.Name = FText::FromString("Hydra | Difficulty");
            Data.Icon = LoadIcon("HydraDifficulty");
            Data.IconColor = FLinearColor(1, 0.5, 0, 1);
            Data.Icon1 = LoadIcon("EnemyHealth");
            Data.IconColor1 = GetIconColor("EnemyHealth");
            Data.Icon2 = LoadIcon("EnemyDamage");
            Data.IconColor2 = GetIconColor("EnemyDamage");
            Data.Icon3 = LoadIcon("LuckIcon");
            Data.IconColor3 = GetIconColor("Luck");
            Data.Descriptions.Add(FText::Format(FText::FromString("<EnemyHealth>{0}%</> <Default>Enemy Health</>"), FText::AsNumber(StatValue)));
            Data.Descriptions.Add(FText::Format(FText::FromString("<EnemyDamage>{0}%</> <Default>Enemy Damage</>"), FText::AsNumber(StatValue2)));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Luck>+{0}</> <Default>Luck</>"), FText::AsNumber(StatValue3)));
        }
        else if (BorderType == "CerberusDifficulty")
        {
            Data.Name = FText::FromString("Cerberus | Difficulty");
            Data.Icon = LoadIcon("CerberusDifficulty");
            Data.IconColor = FLinearColor(1, 0.5, 0, 1);
            Data.Icon1 = LoadIcon("EnemyHealth");
            Data.IconColor1 = GetIconColor("EnemyHealth");
            Data.Icon2 = LoadIcon("EnemyDamage");
            Data.IconColor2 = GetIconColor("EnemyDamage");
            Data.Icon3 = LoadIcon("LuckIcon");
            Data.IconColor3 = GetIconColor("Luck");
            Data.Descriptions.Add(FText::Format(FText::FromString("<EnemyHealth>{0}%</> <Default>Enemy Health</>"), FText::AsNumber(StatValue)));
            Data.Descriptions.Add(FText::Format(FText::FromString("<EnemyDamage>{0}%</> <Default>Enemy Damage</>"), FText::AsNumber(StatValue2)));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Luck>+{0}</> <Default>Luck</>"), FText::AsNumber(StatValue3)));
        }
        else if (BorderType == "CurrentDifficulty")
        {
            Data.Name = FText::FromString(AbilityName + " | Difficulty");
            Data.Icon = LoadIcon(AbilityName + "Difficulty");
            Data.IconColor = FLinearColor(1, 0.5, 0, 1);
            Data.Icon1 = LoadIcon("EnemyHealth");
            Data.IconColor1 = GetIconColor("EnemyHealth");
            Data.Icon2 = LoadIcon("EnemyDamage");
            Data.IconColor2 = GetIconColor("EnemyDamage");
            Data.Icon3 = LoadIcon("LuckIcon");
            Data.IconColor3 = GetIconColor("Luck");
            Data.Descriptions.Add(FText::Format(FText::FromString("<EnemyHealth>{0}%</> <Default>Enemy Health</>"), FText::AsNumber(StatValue)));
            Data.Descriptions.Add(FText::Format(FText::FromString("<EnemyDamage>{0}%</> <Default>Enemy Damage</>"), FText::AsNumber(StatValue2)));
            Data.Descriptions.Add(FText::Format(FText::FromString("<Luck>+{0}</> <Default>Luck</>"), FText::AsNumber(StatValue3)));
            }
        else if (BorderType == "Ability1" || BorderType == "Ability2" || BorderType == "Ability3" || BorderType == "Ability4")
        {
            Data.Icon = nullptr;
            Data.Name = FText::FromString(AbilityName);
            Data.Descriptions.Add(FText::FromString(AbilityDesc));
        }
        else if (BorderType == "AbilityUpgrade")
        {
            Data.Name = FText::FromString("Stat Upgrade");
            Data.Icon = LoadIcon("AbilityIcon");
            Data.IconColor = GetIconColor("EnemyDamage");
            Data.Descriptions.Add(FText::FromString("<Default>Spend points to get additional stats.</>"));
        }

        return Data;
    }

    struct FRarityData
    {
        FString Name;
        FLinearColor Color;
        float Multiplier;
        float Chance;

        FRarityData() {}
        FRarityData(const FString& InName, const FLinearColor& InColor, float InMultiplier, float InChance) : Name(InName), Color(InColor), Multiplier(InMultiplier), Chance(InChance) {}
    };

    static const TMap<FString, FRarityData>& GetRarityDataMap()
    {
        static const TMap<FString, FRarityData> RarityDataMap = {
            { "Common",    FRarityData("Common",    FLinearColor(0.753f, 0.753f, 0.753f, 1), 1.0f, 60.0f) },
            { "Uncommon",  FRarityData("Uncommon",  FLinearColor(0.5f, 1.0f, 0.0f, 1),       2.0f, 20.0f) },
            { "Rare",      FRarityData("Rare",      FLinearColor(0.05f, 0.4f, 1.0f, 1),       4.0f, 12.0f) },
            { "Epic",      FRarityData("Epic",      FLinearColor(0.4f, 0.0f, 0.8f, 1),       7.0f, 6.0f) },
            { "Legendary", FRarityData("Legendary", FLinearColor(0.9f, 0.4f, 0.0f, 1),       10.0f, 1.5f) },
            { "Mythic",    FRarityData("Mythic",    FLinearColor(0.745f, 0.0f, 0.314f, 1),   15.0f, 0.4f) },
            { "Relic",     FRarityData("Relic",     FLinearColor(0.0f, 0.76f, 0.76f, 1),     20.0f, 0.1f) }
        };
        return RarityDataMap;
    }

    static FString GetRandomRarity()
    {
        float Roll = FMath::FRandRange(0.f, 100.f);
        float Cumulative = 0.f;

        for (const auto& Pair : GetRarityDataMap())
        {
            Cumulative += Pair.Value.Chance;
            if (Roll <= Cumulative)
            {
                return Pair.Key;
            }
        }

        return "Common";
    }

    static FString GetRandomRarityWithLuck(float OddsIncrease)
    {
        float Roll = FMath::FRandRange(OddsIncrease, 100.f);
        float Cumulative = 0.f;

        for (const auto& Pair : GetRarityDataMap())
        {
            Cumulative += Pair.Value.Chance;
            if (Roll <= Cumulative)
            {
                return Pair.Key;
            }
        }

        return "Common";
    }

    static FString GetRandomItemType()
    {
        static const TArray<FString> ItemTypes = {
            "Helmet", "Arms", "Upper", "Lower", "Necklace"
        };

        return ItemTypes[FMath::RandRange(0, ItemTypes.Num() - 1)];
    }


    static UItemObject* CreateItemFromData(const FItemData& ItemData, UObject* Outer)
    {
        if (!Outer)
        {
            UE_LOG(LogTemp, Error, TEXT("CreateItemFromData called with null Outer!"));
            return nullptr;
        }

        UItemObject* Item = NewObject<UItemObject>(Outer);
        Item->SetItemData(ItemData);
        return Item;
    }

    static FItemData CreateSpecificItemData(int Level, FString ItemType, UItemMeshData* MeshData, UIconLibrary* IconLibrary)
    {
        return CreateSpecificItemData(Level, ItemType, GetRandomRarity(), MeshData, IconLibrary);
    }

    static FItemData CreateSpecificItemData(int Level, FString ItemType, FString Rarity, UItemMeshData* MeshData, UIconLibrary* IconLibrary)
    {
        FItemData ItemData;
        ItemData.Rarity = Rarity;
        ItemData.ItemType = ItemType;
        ItemData.Name = ItemData.ItemType;
        ItemData.Level = Level;

        float BaseMin = 0.1f;
        float BaseMax = 1.f;
        float RarityMult = GetRarityDataMap()[ItemData.Rarity].Multiplier;

        //Base scaling based on Level and Rarity
        float Min = BaseMin * ItemData.Level * RarityMult;
        float Max = BaseMax * ItemData.Level * RarityMult;

        float LevelBaseValue = ItemData.Level * 0.8f;
        float RarityFloor = RarityMult * 1.3f;

        //Gold
        int CalculatedGold = FMath::RoundToInt((LevelBaseValue + RarityFloor) * FMath::FRandRange(0.9f, 1.1f));
        ItemData.Gold = FMath::Max(1, CalculatedGold);

        TSet<FString> ArmorTypes = { "Helmet", "Upper", "Lower", "Arms" };
        TSet<FString> MixedTypes = { "Necklace" };

        //Size of armor multiplier
        float SizeMult = 1.0f;
        if (ItemType == "Upper" || ItemType == "Lower") SizeMult = 1.5f;
        else if (ItemType == "Helmet") SizeMult = 1.3f;
        else if (ItemType == "Arms") SizeMult = 1.1f;

        auto AssignTripleStats = [&](int& Primary, int& Secondary, int& Tertiary)
            {
                TArray<int*> Stats = { &Primary, &Secondary, &Tertiary };

                //Pick random main stat
                for (int i = Stats.Num() - 1; i > 0; i--)
                {
                    int j = FMath::RandRange(0, i);
                    Stats.Swap(i, j);
                }

                //Apply weights
                float SecondaryWeight = FMath::FRandRange(0.4f, 0.77f);
                float TertiaryWeight = FMath::FRandRange(0.0f, 0.33f);
                *Stats[0] = FMath::Max(0, FMath::RoundToInt(FMath::FRandRange(Min, Max)));
                *Stats[1] = FMath::Max(0, FMath::RoundToInt(FMath::FRandRange(Min * SecondaryWeight, Max * SecondaryWeight)));
                *Stats[2] = FMath::Max(0, FMath::RoundToInt(FMath::FRandRange(Min * TertiaryWeight, Max * TertiaryWeight)));
            };

        if (ArmorTypes.Contains(ItemData.ItemType))
        {
            ItemData.Health = FMath::Max(0, FMath::RoundToInt((ItemData.Level + FMath::FRandRange(Min, Max)) * SizeMult));
            ItemData.Armor = FMath::Max(0, FMath::RoundToInt((ItemData.Level + FMath::FRandRange(Min, Max))));

            AssignTripleStats(ItemData.Strength, ItemData.Dexterity, ItemData.Intelligence);
        }
        else if (MixedTypes.Contains(ItemData.ItemType))
        {
            AssignTripleStats(ItemData.Strength, ItemData.Dexterity, ItemData.Intelligence);
        }

        if (MeshData) { SetItemMeshes(ItemData, MeshData); }

        if (IconLibrary)
        {
            FName KeyName = FName(*ItemData.ItemType);
            UTexture2D** FoundTexturePtr = IconLibrary->IconMap.Find(KeyName);

            if (FoundTexturePtr && *FoundTexturePtr)
            {
                ItemData.Icon = *FoundTexturePtr;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Icon not found in Library for Type: %s"), *ItemData.ItemType);
            }
        }

        ItemData.PowerLevel = ItemData.Health + ItemData.Armor + ItemData.Dexterity + ItemData.Intelligence + ItemData.Strength;

        return ItemData;
    }

    static UTexture2D* LoadIcon(const FString& IconName)
    {
        FString Path = FString::Printf(TEXT("Texture2D'/Game/ThirdPerson/Icons/%s.%s'"), *IconName, *IconName);

        UTexture2D* LoadedTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *Path));

        if (!LoadedTexture)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to load icon texture at path: %s"), *Path);
        }

        return LoadedTexture;
    }

    static FLinearColor GetIconColor(FString StatName)
    {
        FLinearColor Color;

        //Basic Stats
        if (StatName == "Health") Color = FLinearColor(1, 0, 0.02f, 1);
        if (StatName == "Defence") Color = FLinearColor(0, 1, 0.026f, 1);
        if (StatName == "Strength") Color = FLinearColor(0.46f, 0, 1, 1);
        if (StatName == "Agility") Color = FLinearColor(1, 0.855f, 0, 1);
        if (StatName == "Intelligence") Color = FLinearColor(0, 0.723f, 1, 1);

        //Meta Stats
        if (StatName == "Luck") Color = FLinearColor(0, 0.35f, 0.237f, 1);
        if (StatName == "Level") Color = FLinearColor(1, 1, 1, 1);
        if (StatName == "MetaLevel") Color = FLinearColor(0, 0.171, 0.413, 1);
        if (StatName == "Mana") Color = FLinearColor(0, 0.723f, 1, 1);
        if (StatName == "Purity") Color = FLinearColor(1, 0, 0.02f, 1);

        //Enemy Stats
        if (StatName == "EnemyHealth") Color = FLinearColor(0.776f, 0.061f, 0, 1);
        if (StatName == "EnemyDamage") Color = FLinearColor(1, 1, 1, 1);

        return Color;
    }

    static float RoundToDecimal(float Value, int Decimals)
    {
        float Multiplier = FMath::Pow(10.0f, Decimals); 
        return FMath::RoundToFloat(Value * Multiplier) / Multiplier;
    }

    static float GetLootQuality(int Value)
    {
        float Percent = Value * 2.3f;

        return RoundToDecimal(Percent, 1);
    }
    static float GetLootChance(int Value)
    {
        float Percent = Value * 1.45;

        return RoundToDecimal(Percent, 1);
    }

    static float GetHealPercent(int Value)
    {
        float Percent = Value;

        return RoundToDecimal(Percent, 1);
    }

    static float GetChanceToNotConsume(int Value)
    {
        float Percent = Value / 2.9;

        return RoundToDecimal(Percent, 1);
    }

    static float GetDefencePerc(int Value)
    {
        //The constant K determines how fast the curve climbs.
        //At Value = 100, the result will be half of max.
        const float K = 300.0f;
        const float MaxPercent = 60.0f;

        if (Value <= 0) return 0.0f;

        float Percent = (Value / (Value + K)) * MaxPercent;

        return RoundToDecimal(Percent, 1);
    }

    static float GetStrengthPerc(int Value)
    {
        if (Value <= 0) return 0.0f;

        //Base of 1.1 with a 0.9 exponent.
        //This grows infinitely but slows down over time.
        float Percent = 1.1f * pow(static_cast<float>(Value), 0.9f);

        return RoundToDecimal(Percent, 1);
    }

    static float GetSpeedPerc(int Value)
    {
        //The constant K determines how fast the curve climbs.
        //At Value = 100, the result will be half of max.
        const float K = 200.0f;
        const float MaxPercent = 75.0f;

        if (Value <= 0) return 0.0f;

        float Percent = (Value / (Value + K)) * MaxPercent;

        return RoundToDecimal(Percent, 1);
    }

    static float GetCritPerc(int Value)
    {
        //The constant K determines how fast the curve climbs.
        //At Value = 100, the result will be half of max.
        const float K = 200.0f;
        const float MaxPercent = 80.0f;

        if (Value <= 0) return 0.0f;

        float Percent = (Value / (Value + K)) * MaxPercent;

        return RoundToDecimal(Percent, 1);
    }

    static float GetCritDmgPerc(int Value)
    {
        float Percent = 1.5 + (Value / 100.0f);

        return RoundToDecimal(Percent, 1);
    }

    static int GetManaPoolIncrease(int Value)
    {
        int Integer = Value / 2.1;

        return Integer;
    }

    static float GetAbilityCostPerc(int Value)
    {
        //The constant K determines how fast the curve climbs.
        //At Value = 100, the result will be half of max.
        const float K = 200.0f;
        const float MaxPercent = 70.0f;

        if (Value <= 0) return 0.0f;

        float Percent = (static_cast<float>(Value) / (Value + K)) * MaxPercent;

        return RoundToDecimal(Percent, 1);
    }

    static FForgeResultOdds CalculateForgeOdds(const FString& LeftRarity, const FString& RightRarity)
    {
        static const TArray<FString> RarityOrder = { "Common", "Uncommon", "Rare", "Epic", "Legendary", "Mythic", "Relic" };
        static const float UpgradeSameTier[7] = { 90.f, 80.f, 70.f, 60.f, 50.f, 40.f, 0.f };

        auto GetIndex = [&](const FString& R) {
            int FoundIndex = RarityOrder.IndexOfByKey(R);
            return (FoundIndex == INDEX_NONE) ? 0 : FoundIndex;
            };

        int IndexA = GetIndex(LeftRarity);
        int IndexB = GetIndex(RightRarity);

        int Highest = FMath::Max(IndexA, IndexB);
        int Lowest = FMath::Min(IndexA, IndexB);
        int Distance = FMath::Max(1, Highest - Lowest);

        FForgeResultOdds OutOdds;

        int UpIndex = FMath::Min(Highest + 1, RarityOrder.Num() - 1);
        int MidIndex = Highest;
        int DownIndex = FMath::Max(Highest - 1, 0);

        OutOdds.Rarity1 = RarityOrder[DownIndex]; 
        OutOdds.Rarity2 = RarityOrder[MidIndex];  
        OutOdds.Rarity3 = RarityOrder[UpIndex];   

        float UpChance = 0.f;
        int TierIndex = FMath::Clamp(Highest, 0, 6);

        if (IndexA == IndexB) {
            UpChance = UpgradeSameTier[TierIndex];
        }
        else {
            float Proximity = 1.f / float(Distance);
            UpChance = UpgradeSameTier[TierIndex] * Proximity * 0.8f;
        }

        float Remaining = 100.0f - UpChance;

        OutOdds.Chance3 = UpChance;
        OutOdds.Chance2 = Remaining * 0.7f;
        OutOdds.Chance1 = Remaining * 0.3f;

        //Special Case: Two Commons
        if (Highest == 0) {
            OutOdds.Chance2 += OutOdds.Chance1;
            OutOdds.Chance1 = 0.f;
        }

        return OutOdds;
    }

    static FString RollForgeRarity(const FForgeResultOdds& Odds)
    {
        float Roll = FMath::FRandRange(0.f, 100.f);

        UE_LOG(LogTemp, Warning, TEXT("Forge Roll: %.2f | Odds: O1:%.1f O2:%.1f O3:%.1f"),
            Roll, Odds.Chance1, Odds.Chance2, Odds.Chance3);

        if (Roll <= Odds.Chance1)
        {
            return Odds.Rarity1;
        }

        if (Roll <= (Odds.Chance1 + Odds.Chance2))
        {
            return Odds.Rarity2;
        }

        return Odds.Rarity3;
    }

    static FItemData CreateRandomItemData(UItemMeshData* MeshData, UIconLibrary* IconLibrary)
    {
        return CreateSpecificItemData(FMath::RandRange(1,100), GetRandomItemType(), MeshData, IconLibrary);
    }

    static void SetItemMeshes(FItemData& ItemData, UItemMeshData* MeshData)
    {
        if (!MeshData) return;

        const TMap<FString, UStaticMesh*> MeshMap = {
            { "Greatsword", MeshData->GreatswordMesh },
            { "Bow", MeshData->BowMesh },
            { "Staff", MeshData->StaffMesh },
            { "Sword", MeshData->SwordMesh },
            { "Helmet", MeshData->HelmetMesh },
            { "Upper", MeshData->UpperMesh },
            { "Lower", MeshData->LowerMesh },
            { "Arms", MeshData->ArmsMesh },
            { "Offhand", MeshData->ShieldMesh },
            { "Necklace", MeshData->NecklaceMesh },
        };

        ItemData.ItemStaticMesh = MeshMap.Contains(ItemData.ItemType) ? MeshMap[ItemData.ItemType] : nullptr;
    }
};
