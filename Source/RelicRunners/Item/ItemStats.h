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

class RELICRUNNERS_API ItemStats
{
public:

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
            { "Rare",      FRarityData("Rare",      FLinearColor(0.0f, 0.3f, 0.9f, 1),       4.0f, 12.0f) },
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

    static FItemData CreateSpecificItemData(int Level, FString ItemType, UItemMeshData* MeshData)
    {
        return CreateSpecificItemData(Level, ItemType, GetRandomRarity(), MeshData);
    }

    static FItemData CreateSpecificItemData(int Level, FString ItemType, FString Rarity, UItemMeshData* MeshData)
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

        TSet<FString> ArmorTypes = { "Helmet", "Upper", "Lower", "Arms" };
        TSet<FString> MixedTypes = { "Necklace" };

        //Size of armor multiplier
        float SizeMult = 1.0f;
        if (ItemType == "Upper" || ItemType == "Lower") SizeMult = 1.5f;
        else if (ItemType == "Helmet") SizeMult = 1.25f;
        else if (ItemType == "Arms") SizeMult = 1.0f;

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
            ItemData.Armor = FMath::Max(0, FMath::RoundToInt((ItemData.Level + FMath::FRandRange(Min, Max)) * SizeMult));

            AssignTripleStats(ItemData.Strength, ItemData.Dexterity, ItemData.Intelligence);

            ItemData.Gold = FMath::Max(1, FMath::RoundToInt(FMath::FRandRange(Min * 0.5f, Max * 0.5f)));
        }
        else if (MixedTypes.Contains(ItemData.ItemType))
        {
            AssignTripleStats(ItemData.Strength, ItemData.Dexterity, ItemData.Intelligence);

            ItemData.Gold = FMath::Max(1, FMath::RoundToInt(FMath::FRandRange(Min, Max)));
        }

        if (MeshData) { SetItemMeshes(ItemData, MeshData); }
        FString ImagePath = FString::Printf(TEXT("Texture2D'/Game/ThirdPerson/Icons/%s.%s'"), *ItemData.ItemType, *ItemData.ItemType);
        if (UTexture2D* Texture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *ImagePath)))
        {
            ItemData.Icon = Texture;
        }

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

    static float GetDefencePerc(int Value)
    {
        float Percent = Value / 2.5;

        return RoundToDecimal(Percent, 1);
    }

    static float GetStrengthPerc(int Value)
    {
        float Percent = Value / 1.75;

        return RoundToDecimal(Percent, 1);
    }

    static float GetSpeedPerc(int Value)
    {
        float Percent = Value / 4.8;

        return RoundToDecimal(Percent, 1);
    }

    static float GetCritPerc(int Value)
    {
        float Percent = Value / 2.3;

        return RoundToDecimal(Percent, 1);
    }

    static float GetCritDmgPerc(int Value)
    {
        float Percent = 1.5 + (Value / 100);

        return RoundToDecimal(Percent, 1);
    }

    static int GetManaPoolIncrease(int Value)
    {
        int Integer = Value / 2.1;

        return Integer;
    }

    static float GetAbilityCostPerc(int Value)
    {
        float Percent = Value / 2.7;

        return RoundToDecimal(Percent, 1);
    }

    static FItemData CreateRandomItemData(UItemMeshData* MeshData)
    {
        return CreateSpecificItemData(FMath::RandRange(1,100), GetRandomItemType(), MeshData);
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
