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
#include "Engine/DataTable.h"
#include "ItemData.generated.h"

USTRUCT(BlueprintType)
struct RELICRUNNERS_API FItemData
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString ItemType;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString Name;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString Rarity;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int Level = 1;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int Health = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int Armor = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int Dexterity = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int Strength = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int Intelligence = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int Gold = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    UStaticMesh* ItemStaticMesh = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FGuid UniqueID = FGuid::NewGuid();

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    UTexture2D* Icon = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FText Description;
};

UCLASS(BlueprintType)
class RELICRUNNERS_API UItemObject : public UObject
{
    GENERATED_BODY()

public:

    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere)
    FItemData ItemData;

    UFUNCTION(BlueprintCallable)
    FString GetItemType() const { return ItemData.ItemType; }

    UFUNCTION(BlueprintCallable)
    FString GetName() const { return ItemData.Name; }

    UFUNCTION(BlueprintCallable)
    FString GetRarity() const { return ItemData.Rarity; }

    UFUNCTION(BlueprintCallable)
    int GetLevel() const { return ItemData.Level; }

    UFUNCTION(BlueprintCallable)
    int GetHealth() const { return ItemData.Health; }

    UFUNCTION(BlueprintCallable)
    int GetArmor() const { return ItemData.Armor; }

    UFUNCTION(BlueprintCallable)
    int GetDexterity() const { return ItemData.Dexterity; }

    UFUNCTION(BlueprintCallable)
    int GetStrength() const { return ItemData.Strength; }

    UFUNCTION(BlueprintCallable)
    int GetIntelligence() const { return ItemData.Intelligence; }

    UFUNCTION(BlueprintCallable)
    FGuid GetUniqueID() const { return ItemData.UniqueID; }

    UFUNCTION(BlueprintCallable)
    UStaticMesh* GetItemStaticMesh() const { return ItemData.ItemStaticMesh; }

    UFUNCTION(BlueprintCallable)
    FItemData GetItemData() const { return ItemData; }

    UFUNCTION(BlueprintCallable)
    void SetItemData(const FItemData& NewData) { ItemData = NewData; }

    bool IsNewItem = false;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual bool IsSupportedForNetworking() const override { return true; }
};
