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
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "RelicRunnersPlayerState.generated.h"

UCLASS()
class RELICRUNNERS_API ARelicRunnersPlayerState : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()
public:
    //Functions
    ARelicRunnersPlayerState();
    ~ARelicRunnersPlayerState();
    UFUNCTION()
    void OnRep_SelectedClass();

    //Properties
    UPROPERTY(ReplicatedUsing = OnRep_SelectedClass)
    FName SelectedClass;

    //Getters and Setters
    UAttributeSet* GetAttributeSet() const { return AttributeSet; }
    void SetSelectedClass(FName NewClass);
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    //Delegate
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnSelectedClassChanged, FName);
    FOnSelectedClassChanged OnSelectedClassChanged;

protected:

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UAttributeSet> AttributeSet;

};

