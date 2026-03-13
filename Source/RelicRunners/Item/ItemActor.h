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
#include "GameFramework/Actor.h"
#include "RelicRunners/Interact/InteractInterface.h"
#include "ItemData.h"
#include "ItemActor.generated.h"

class UStaticMeshComponent;
class ULocalFogVolumeComponent;
class UWidgetComponent;
class UUserWidget;
class UInventoryToolTip;

UCLASS()
class RELICRUNNERS_API AItemActor : public AActor, public IInteractInterface
{
    GENERATED_BODY()

public:

    AItemActor();

    //Functions
    void InitializeTooltipWidget();
    UFUNCTION(BlueprintCallable)
    void Initialize(const FItemData& InItemData);
    void SetupVisuals();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
    class USphereComponent* AutoLootSphere = nullptr;

    UFUNCTION()
    void OnAutoLootSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
    void OnRep_ItemData();
    void HandlePickup(class ARelicRunnersCharacter* Char);

    //Properties
    UPROPERTY()
    UStaticMeshComponent* StaticMeshComponent;
    UPROPERTY(ReplicatedUsing = OnRep_ItemData)
    struct FItemData ItemData;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
    class ULocalFogVolumeComponent* Fog;
    UPROPERTY(VisibleAnywhere)
    class UWidgetComponent* TooltipWidgetComponent;
    UPROPERTY(EditAnywhere)
    TSubclassOf<UUserWidget> TooltipWidgetClass;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ItemCoverMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AutoLootSphereRadius = 40.f;

protected:
    // IInteractInterface overrides
    virtual void ShowTooltip_Implementation(bool bShow) override;
    virtual void Interact_Implementation(class ARelicRunnersCharacter* Char) override;
    virtual struct FItemData GetItemData_Implementation() override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;
};
