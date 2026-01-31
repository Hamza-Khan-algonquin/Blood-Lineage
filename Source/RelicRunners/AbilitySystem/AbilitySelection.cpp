// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySelection.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Inventory/Inventory.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include <RelicRunners/Item/ItemStats.h>


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

    //text display UI on buttons 
    switch (RandomStatBuff.StatType)
    {
    case EStats::Health:
        RandomStatBuff.StatText = FString::Printf(TEXT("+%d"), RandomStatBuff.Percentage);
        RandomStatBuff.Icon = ItemStats::LoadIcon("HealthIcon");
        RandomStatBuff.Color = ItemStats::GetIconColor("Health");
        break;
    case EStats::Armor:
        RandomStatBuff.StatText = FString::Printf(TEXT("+%d"), RandomStatBuff.Percentage);
        RandomStatBuff.Icon = ItemStats::LoadIcon("DefenceIcon");
        RandomStatBuff.Color = ItemStats::GetIconColor("Defence");
        break;
    case EStats::Dexterity:
        RandomStatBuff.StatText = FString::Printf(TEXT("+%d"), RandomStatBuff.Percentage);
        RandomStatBuff.Icon = ItemStats::LoadIcon("AgilityIcon");
        RandomStatBuff.Color = ItemStats::GetIconColor("Agility");
        break;
    case EStats::Intelligence:
        RandomStatBuff.StatText = FString::Printf(TEXT("+%d"), RandomStatBuff.Percentage);
        RandomStatBuff.Icon = ItemStats::LoadIcon("IntelligenceIcon");
        RandomStatBuff.Color = ItemStats::GetIconColor("Intelligence");
        break;
    case EStats::Strength:
        RandomStatBuff.StatText = FString::Printf(TEXT("+%d"), RandomStatBuff.Percentage);
        RandomStatBuff.Icon = ItemStats::LoadIcon("StrengthIcon");
        RandomStatBuff.Color = ItemStats::GetIconColor("Strength");
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
    Icon_0->SetBrushFromTexture(Buff1.Icon);
    Icon_0->SetColorAndOpacity(Buff1.Color);

    Buff2 = RandomStatBuff();
    TB_Upgrade2->SetText(FText::FromString(Buff2.StatText));
    Icon_1->SetBrushFromTexture(Buff2.Icon);
    Icon_1->SetColorAndOpacity(Buff2.Color);

    Buff3 = RandomStatBuff();
    TB_Upgrade3->SetText(FText::FromString(Buff3.StatText));
    Icon_2->SetBrushFromTexture(Buff3.Icon);
    Icon_2->SetColorAndOpacity(Buff3.Color);

    Buff4 = RandomStatBuff();
    TB_Upgrade4->SetText(FText::FromString(Buff4.StatText));
    Icon_3->SetBrushFromTexture(Buff4.Icon);
    Icon_3->SetColorAndOpacity(Buff4.Color);
}


