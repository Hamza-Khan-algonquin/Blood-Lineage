// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySelection.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Inventory/Inventory.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include <RelicRunners/Item/ItemStats.h>
#include "RelicRunners/Menu/StandardTooltip.h"

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

    if (UpgradePickSound)
    {
        UGameplayStatics::PlaySound2D(this, UpgradePickSound);
    }

    PS->Server_SpendAbilityPoints();
    BuffStatValue(Buff1);
    RefreshUpgrades();
}

void UAbilitySelection::Upgrade2Clicked()
{
    ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>();
    if (!PS) return;

    if (UpgradePickSound)
    {
        UGameplayStatics::PlaySound2D(this, UpgradePickSound);
    }

    PS->Server_SpendAbilityPoints();
    BuffStatValue(Buff2);
    RefreshUpgrades();
}

void UAbilitySelection::Upgrade3Clicked()
{
    ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>();
    if (!PS) return;

    if (UpgradePickSound)
    {
        UGameplayStatics::PlaySound2D(this, UpgradePickSound);
    }

    PS->Server_SpendAbilityPoints();
    BuffStatValue(Buff3);
    RefreshUpgrades();
}

void UAbilitySelection::Upgrade4Clicked()
{
    ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>();
    if (!PS) return;

    if (UpgradePickSound)
    {
        UGameplayStatics::PlaySound2D(this, UpgradePickSound);
    }

    PS->Server_SpendAbilityPoints();
    BuffStatValue(Buff4);
    RefreshUpgrades();
}

void UAbilitySelection::BuffStatValue(StatBuff Buff)
{
    ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>();
    if (!PS) return;

    if (Buff.StatType == EStats::Health)
        PS->UpgradedHealth += Buff.Percentage;
    if (Buff.StatType == EStats::Intelligence)
        PS->UpgradedIntelligence += Buff.Percentage;
    if (Buff.StatType == EStats::Strength)
        PS->UpgradedStrength += Buff.Percentage;
    if (Buff.StatType == EStats::Armor)
        PS->UpgradedArmor += Buff.Percentage;
    if (Buff.StatType == EStats::Dexterity)
        PS->UpgradedDexterity += Buff.Percentage;
}

UAbilitySelection::StatBuff UAbilitySelection::RandomStatBuff(const TArray<EStats>& ExcludedStats)
{
    StatBuff NewBuff;
    NewBuff.Percentage = FMath::RandRange(3, 6);

    TArray<EStats> AvailableStats = { EStats::Health, EStats::Armor, EStats::Dexterity, EStats::Intelligence, EStats::Strength };

    for (EStats Excluded : ExcludedStats)
    {
        AvailableStats.Remove(Excluded);
    }

    if (AvailableStats.Num() > 0)
    {
        int RandomIndex = FMath::RandRange(0, AvailableStats.Num() - 1);
        NewBuff.StatType = AvailableStats[RandomIndex];
    }

    switch (NewBuff.StatType)
    {
    case EStats::Health:
        NewBuff.StatText = FString::Printf(TEXT("+%d"), NewBuff.Percentage);
        NewBuff.Icon = ItemStats::LoadIcon("HealthIcon");
        NewBuff.Color = ItemStats::GetIconColor("Health");
        break;
    case EStats::Armor:
        NewBuff.StatText = FString::Printf(TEXT("+%d"), NewBuff.Percentage);
        NewBuff.Icon = ItemStats::LoadIcon("DefenceIcon");
        NewBuff.Color = ItemStats::GetIconColor("Defence");
        break;
    case EStats::Dexterity:
        NewBuff.StatText = FString::Printf(TEXT("+%d"), NewBuff.Percentage);
        NewBuff.Icon = ItemStats::LoadIcon("AgilityIcon");
        NewBuff.Color = ItemStats::GetIconColor("Agility");
        break;
    case EStats::Intelligence:
        NewBuff.StatText = FString::Printf(TEXT("+%d"), NewBuff.Percentage);
        NewBuff.Icon = ItemStats::LoadIcon("IntelligenceIcon");
        NewBuff.Color = ItemStats::GetIconColor("Intelligence");
        break;
    case EStats::Strength:
        NewBuff.StatText = FString::Printf(TEXT("+%d"), NewBuff.Percentage);
        NewBuff.Icon = ItemStats::LoadIcon("StrengthIcon");
        NewBuff.Color = ItemStats::GetIconColor("Strength");
        break;
    default:
        break;
    }

    return NewBuff;
}

void UAbilitySelection::RefreshUpgrades()
{
    ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>();
    if (PS && PS->GetInventoryComponent())
    {
        PS->GetInventoryComponent()->UpdateTotalEquippedStats(PS->GetPawn());
    }

    TArray<EStats> UsedStats;

    Buff1 = RandomStatBuff(UsedStats);
    UsedStats.Add(Buff1.StatType);
    TB_Upgrade1->SetText(FText::FromString(Buff1.StatText));
    Icon_0->SetBrushFromTexture(Buff1.Icon);
    Icon_0->SetColorAndOpacity(Buff1.Color);

    Buff2 = RandomStatBuff(UsedStats);
    UsedStats.Add(Buff2.StatType);
    TB_Upgrade2->SetText(FText::FromString(Buff2.StatText));
    Icon_1->SetBrushFromTexture(Buff2.Icon);
    Icon_1->SetColorAndOpacity(Buff2.Color);

    Buff3 = RandomStatBuff(UsedStats);
    UsedStats.Add(Buff3.StatType);
    TB_Upgrade3->SetText(FText::FromString(Buff3.StatText));
    Icon_2->SetBrushFromTexture(Buff3.Icon);
    Icon_2->SetColorAndOpacity(Buff3.Color);

    Buff4 = RandomStatBuff(UsedStats);
    UsedStats.Add(Buff4.StatType);
    TB_Upgrade4->SetText(FText::FromString(Buff4.StatText));
    Icon_3->SetBrushFromTexture(Buff4.Icon);
    Icon_3->SetColorAndOpacity(Buff4.Color);
}

FReply UAbilitySelection::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseMove(InGeometry, InMouseEvent);

    if (!IsVisible()) return FReply::Handled();

    FVector2D CursorPos = InMouseEvent.GetScreenSpacePosition();

    TArray<UWidget*> Hoverables = {
        B_Upgrade1, B_Upgrade2, B_Upgrade3, B_Upgrade4,
        Icon_0, Icon_1, Icon_2, Icon_3
    };

    UWidget* FoundHoveredWidget = nullptr;
    for (UWidget* Widget : Hoverables)
    {
        if (!Widget || !Widget->IsVisible()) continue;

        FGeometry WidgetGeom = Widget->GetCachedGeometry();
        if (WidgetGeom.IsUnderLocation(CursorPos))
        {
            FoundHoveredWidget = Widget;
            break;
        }
    }

    if (FoundHoveredWidget != CurrentHoveredWidget)
    {
        if (StandardTooltip)
        {
            StandardTooltip->RemoveFromParent();
            StandardTooltip = nullptr;
        }
        CurrentHoveredWidget = FoundHoveredWidget;
    }

    if (CurrentHoveredWidget)
    {
        ShowTooltipForWidget(CurrentHoveredWidget, CursorPos, InGeometry);
        return FReply::Handled();
    }

    return FReply::Unhandled();
}

void UAbilitySelection::ShowTooltipForWidget(UWidget* HoveredWidget, const FVector2D& CursorPos, const FGeometry& ParentGeometry)
{
    if (!StandardTooltipClass || !HoveredWidget) return;

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;
    ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(PC);
    if (!RPC) return;
    ARelicRunnersPlayerState* RPS = Cast<ARelicRunnersPlayerState>(RPC->PlayerState);
    if (!RPS) return;

    if (!StandardTooltip)
    {
        StandardTooltip = CreateWidget<UStandardTooltip>(PC, StandardTooltipClass);
        StandardTooltip->AddToViewport();
    }

    FString StatKey = "";
    float RawValue = 0;

    StatBuff* TargetBuff = nullptr;

    if (HoveredWidget == B_Upgrade1 || HoveredWidget == Icon_0) TargetBuff = &Buff1;
    else if (HoveredWidget == B_Upgrade2 || HoveredWidget == Icon_1) TargetBuff = &Buff2;
    else if (HoveredWidget == B_Upgrade3 || HoveredWidget == Icon_2) TargetBuff = &Buff3;
    else if (HoveredWidget == B_Upgrade4 || HoveredWidget == Icon_3) TargetBuff = &Buff4;

    if (TargetBuff)
    {
        switch(TargetBuff->StatType)
        {
            case EStats::Health: StatKey = "Health"; break;
            case EStats::Armor: StatKey = "Defence"; break;
            case EStats::Dexterity: StatKey = "Agility"; break;
            case EStats::Intelligence: StatKey = "Intelligence"; break;
            case EStats::Strength: StatKey = "Strength"; break;
        }
        RawValue = TargetBuff->Percentage;
    }

    FVector2D Offset(0, 50.0f);
    FVector2D Anchor(0.5f, 0.0f); 
    
    FTooltipData TD = ItemStats::GetStatTooltipData(StatKey, RawValue, 0, 0, "", "");

    FVector2D ViewportPosition = ParentGeometry.AbsoluteToLocal(CursorPos);
    StandardTooltip->SetAnchor(Anchor);
    StandardTooltip->SetIcon(TD.Icon, TD.IconColor, 0);
    StandardTooltip->Setup(TD.Name, TD.Value, TD.Descriptions);
    StandardTooltip->SetPositionInViewport(ViewportPosition + Offset, false);
}