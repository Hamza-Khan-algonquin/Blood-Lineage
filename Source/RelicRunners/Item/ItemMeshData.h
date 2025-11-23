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
#include "Engine/DataAsset.h"
#include "ItemMeshData.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UItemMeshData : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, Category = "Meshes") UStaticMesh* SwordMesh;
    UPROPERTY(EditDefaultsOnly, Category = "Meshes") UStaticMesh* GreatswordMesh;
    UPROPERTY(EditDefaultsOnly, Category = "Meshes") UStaticMesh* BowMesh;
    UPROPERTY(EditDefaultsOnly, Category = "Meshes") UStaticMesh* StaffMesh;
    UPROPERTY(EditDefaultsOnly, Category = "Meshes") UStaticMesh* ShieldMesh;
    UPROPERTY(EditDefaultsOnly, Category = "Meshes") UStaticMesh* LowerMesh;
    UPROPERTY(EditDefaultsOnly, Category = "Meshes") UStaticMesh* UpperMesh;
    UPROPERTY(EditDefaultsOnly, Category = "Meshes") UStaticMesh* NecklaceMesh;
    UPROPERTY(EditDefaultsOnly, Category = "Meshes") UStaticMesh* HelmetMesh;
    UPROPERTY(EditDefaultsOnly, Category = "Meshes") UStaticMesh* ArmsMesh;
};
