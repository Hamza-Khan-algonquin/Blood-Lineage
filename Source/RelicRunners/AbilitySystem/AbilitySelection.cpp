// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySelection.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Inventory/Inventory.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"


void UAbilitySelection::NativeConstruct()
{
    Super::NativeConstruct();

    //Bind upgrade button clicks
    B_Upgrade1->OnClicked.AddDynamic(this, &UAbilitySelection::Upgrade1Clicked);
    B_Upgrade2->OnClicked.AddDynamic(this, &UAbilitySelection::Upgrade2Clicked);
    B_Upgrade3->OnClicked.AddDynamic(this, &UAbilitySelection::Upgrade3Clicked);
    B_Upgrade4->OnClicked.AddDynamic(this, &UAbilitySelection::Upgrade4Clicked);

    //Get player reference
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        APawn* Pawn = PC->GetPawn();
        OwningCharacter = Cast<ARelicRunnersCharacter>(Pawn);
    }

	//Show fresh upgrade options
    RefreshUpgrades();
}

void UAbilitySelection::Upgrade1Clicked()
{
    ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>();
    if (!PS) return;

    PS->Server_SpendAbilityPoints();
    BuffStatValue(Buff1);
    RefreshUpgrades();
}

void UAbilitySelection::Upgrade2Clicked()
{
    ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>();
    if (!PS) return;

    PS->Server_SpendAbilityPoints();
    BuffStatValue(Buff2);
    RefreshUpgrades();
}

void UAbilitySelection::Upgrade3Clicked()
{
    ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>();
    if (!PS) return;

    PS->Server_SpendAbilityPoints();
    BuffStatValue(Buff3);
    RefreshUpgrades();
}

void UAbilitySelection::Upgrade4Clicked()
{
    ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>();
    if (!PS) return;

    PS->Server_SpendAbilityPoints();
    BuffStatValue(Buff4);
    RefreshUpgrades();
}

void UAbilitySelection::BuffStatValue(StatBuff Buff)
{
    ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>();
    if (!PS) return;

    if (Buff.StatType == EStats::Health)
        PS->SetPlayerStartingHealth(PS->GetPlayerStartingMaxHealth() + Buff.Percentage);
    if (Buff.StatType == EStats::Intelligence)
        PS->SetPlayerStartingIntelligence(PS->GetPlayerStartingIntelligence() + Buff.Percentage);
    if (Buff.StatType == EStats::Strength)
        PS->SetPlayerStartingStrength(PS->GetPlayerStartingStrength() + Buff.Percentage);
    if (Buff.StatType == EStats::Armor)
        PS->SetPlayerStartingArmor(PS->GetPlayerStartingArmor() + Buff.Percentage);
    if (Buff.StatType == EStats::Dexterity)
        PS->SetPlayerStartingDexterity(PS->GetPlayerStartingDexterity() + Buff.Percentage);
}

UAbilitySelection::StatBuff UAbilitySelection::RandomStatBuff()
{
    StatBuff RandomStatBuff;

    //Pick random stat and buff value
    RandomStatBuff.Percentage = FMath::RandRange(3, 5);
    RandomStatBuff.StatType = static_cast<UAbilitySelection::EStats>(FMath::RandRange(0, 4));
    RandomStatBuff.StatImage = nullptr;

    //text display UI on buttons 
    switch (RandomStatBuff.StatType)
    {
    case EStats::Health:
        RandomStatBuff.StatText = FString::Printf(TEXT("+%d Health"), RandomStatBuff.Percentage);
        break;
    case EStats::Armor:
        RandomStatBuff.StatText = FString::Printf(TEXT("+%d Armor"), RandomStatBuff.Percentage);
        break;
    case EStats::Dexterity:
        RandomStatBuff.StatText = FString::Printf(TEXT("+%d Dexterity"), RandomStatBuff.Percentage);
        break;
    case EStats::Intelligence:
        RandomStatBuff.StatText = FString::Printf(TEXT("+%d Intelligence"), RandomStatBuff.Percentage);
        break;
    case EStats::Strength:
        RandomStatBuff.StatText = FString::Printf(TEXT("+%d Strength"), RandomStatBuff.Percentage);
        break;
    default:
        break;
    }

    return RandomStatBuff;
}

void UAbilitySelection::RefreshUpgrades()
{
    ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>();
    if (PS && PS->GetInventoryComponent())
    {
        PS->GetInventoryComponent()->UpdateTotalEquippedStats(PS->GetPawn());
    }

    Buff1 = RandomStatBuff();
    TB_Upgrade1->SetText(FText::FromString(Buff1.StatText));

    Buff2 = RandomStatBuff();
    TB_Upgrade2->SetText(FText::FromString(Buff2.StatText));

    Buff3 = RandomStatBuff();
    TB_Upgrade3->SetText(FText::FromString(Buff3.StatText));

    Buff4 = RandomStatBuff();
    TB_Upgrade4->SetText(FText::FromString(Buff4.StatText));
}


