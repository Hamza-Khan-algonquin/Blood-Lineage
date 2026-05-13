#include "UpgradesWidget.h"
#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Textblock.h"
#include "Components/ProgressBar.h"
#include "Components/RadialSlider.h"
#include "RelicRunners/Item/ItemStats.h"
#include "RelicRunners/Game/RelicRunnersGameInstance.h"
#include "RelicRunners/Menu/SaveFileListData.h"
#include "RelicRunners/Menu/SaveFileListWidget.h"
#include "RelicRunners/Menu/StandardTooltip.h"
#include "Kismet/GameplayStatics.h"
#include <RelicRunners/PlayerState/RelicRunnersPlayerState.h>

void UUpgradesWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackButton) BackButton->OnClicked.AddDynamic(this, &UUpgradesWidget::BackButtonClicked);

	//Setup Icons Colors
	if (IconHP)		IconHP->SetColorAndOpacity(ItemStats::GetIconColor("Health"));
	if (IconDEF)	IconDEF->SetColorAndOpacity(ItemStats::GetIconColor("Defence"));
	if (IconLUCK)	IconLUCK->SetColorAndOpacity(ItemStats::GetIconColor("Luck"));
	if (IconSTR)	IconSTR->SetColorAndOpacity(ItemStats::GetIconColor("Strength"));
	if (IconAGI)	IconAGI->SetColorAndOpacity(ItemStats::GetIconColor("Agility"));
	if (IconINT)	IconINT->SetColorAndOpacity(ItemStats::GetIconColor("Intelligence"));
	if (IconRegen)	IconRegen->SetColorAndOpacity(ItemStats::GetIconColor("Mana"));
	if (IconLevel)	IconLevel->SetColorAndOpacity(ItemStats::GetIconColor("Level"));
	if (IconPurity)	IconPurity->SetColorAndOpacity(ItemStats::GetIconColor("Purity"));

	if (B_HP)			B_HP->OnClicked.AddDynamic(this, &UUpgradesWidget::OnHPClicked);
	if (B_DEF)			B_DEF->OnClicked.AddDynamic(this, &UUpgradesWidget::OnDEFClicked);
	if (B_LUCK)			B_LUCK->OnClicked.AddDynamic(this, &UUpgradesWidget::OnLUCKClicked);
	if (B_STR)			B_STR->OnClicked.AddDynamic(this, &UUpgradesWidget::OnSTRClicked);
	if (B_AGI)			B_AGI->OnClicked.AddDynamic(this, &UUpgradesWidget::OnAGIClicked);
	if (B_INT)			B_INT->OnClicked.AddDynamic(this, &UUpgradesWidget::OnINTClicked);
	if (B_Regen)		B_Regen->OnClicked.AddDynamic(this, &UUpgradesWidget::OnRegenClicked);
	if (B_Level)		B_Level->OnClicked.AddDynamic(this, &UUpgradesWidget::OnLevelClicked);
	if (B_Efficiency)	B_Efficiency->OnClicked.AddDynamic(this, &UUpgradesWidget::OnEfficiencyClicked);

	if (B_ResetHP)         B_ResetHP->OnClicked.AddDynamic(this, &UUpgradesWidget::OnResetHPClicked);
	if (B_ResetDEF)        B_ResetDEF->OnClicked.AddDynamic(this, &UUpgradesWidget::OnResetDEFClicked);
	if (B_ResetLUCK)       B_ResetLUCK->OnClicked.AddDynamic(this, &UUpgradesWidget::OnResetLUCKClicked);
	if (B_ResetSTR)        B_ResetSTR->OnClicked.AddDynamic(this, &UUpgradesWidget::OnResetSTRClicked);
	if (B_ResetAGI)        B_ResetAGI->OnClicked.AddDynamic(this, &UUpgradesWidget::OnResetAGIClicked);
	if (B_ResetINT)        B_ResetINT->OnClicked.AddDynamic(this, &UUpgradesWidget::OnResetINTClicked);
	if (B_ResetRegen)      B_ResetRegen->OnClicked.AddDynamic(this, &UUpgradesWidget::OnResetRegenClicked);
	if (B_ResetLevel)      B_ResetLevel->OnClicked.AddDynamic(this, &UUpgradesWidget::OnResetLevelClicked);
	if (B_ResetEfficiency) B_ResetEfficiency->OnClicked.AddDynamic(this, &UUpgradesWidget::OnResetEfficiencyClicked);
	
}

void UUpgradesWidget::InitializeUpgradeData(USaveFileListData* InSaveData)
{
	CurrentSaveData = InSaveData;
	RefreshUI();
}

void UUpgradesWidget::ShowTooltipForBorder(UWidget* HoveredWidget, const FVector2D& CursorPos, const FGeometry& ParentGeometry)
{
	if (!StandardTooltipClass || !HoveredWidget) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC || !CurrentSaveData) return;

	FSaveData& SD = CurrentSaveData->SaveData;

	if (!StandardTooltip)
	{
		StandardTooltip = CreateWidget<UStandardTooltip>(PC, StandardTooltipClass);
		StandardTooltip->AddToViewport();
	}

	FString StatKey = "";
	float RawValue = 0;

	FVector2D ViewportPosition = ParentGeometry.AbsoluteToLocal(CursorPos);
	StandardTooltip->SetAnchor(FVector2D(0.f, 1.f));
	StandardTooltip->SetPositionInViewport(ViewportPosition + FVector2D(5.0f, 5.0f), false);

	if (HoveredWidget == B_HP) { StatKey = "MetaHealth"; RawValue = SD.MetaHealth * 15; }
	else if (HoveredWidget == B_DEF) { StatKey = "MetaDefence"; RawValue = SD.MetaDefence * 5; }
	else if (HoveredWidget == B_STR) { StatKey = "MetaStrength"; RawValue = SD.MetaStrength * 5; }
	else if (HoveredWidget == B_AGI) { StatKey = "MetaAgility"; RawValue = SD.MetaAgility * 5; }
	else if (HoveredWidget == B_INT) { StatKey = "MetaIntelligence"; RawValue = SD.MetaIntelligence * 5; }
	else if (HoveredWidget == B_Efficiency) { StatKey = "MetaPurity"; RawValue = SD.MetaPotionEfficiency * 5; }
	else if (HoveredWidget == B_LUCK) { StatKey = "MetaLuck"; RawValue = SD.MetaLuck * 1; }
	else if (HoveredWidget == B_Regen) { StatKey = "MetaMana"; RawValue = SD.MetaResourceRegen * 0.5; }
	else if (HoveredWidget == B_Level) { StatKey = "MetaStartingLevel"; RawValue = SD.MetaStartingLevel + 1; }

	else if (HoveredWidget == B_MetaLevel)
	{
		StatKey = "MetaLevelMenu";
		RawValue = SD.MetaLevel;
		StandardTooltip->SetAnchor(FVector2D(0.f, 0.f));
	}
	else if (HoveredWidget == B_MetaPoints)
	{
		StatKey = "MetaPoints";
		RawValue = SD.MetaPoints;
		StandardTooltip->SetAnchor(FVector2D(0.f, 0.f));
	}

	FTooltipData TD;
	if (HoveredWidget == B_Regen)
	{
		if (ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>())
		{
			TD = ItemStats::GetStatTooltipData(StatKey, RawValue, RawValue, PS->PassiveResourceTime, "", "");
		}
	}
	else
	{
		TD = ItemStats::GetStatTooltipData(StatKey, RawValue, 0, 0, "", "");
	}

	StandardTooltip->SetIcon(TD.Icon, TD.IconColor, 0);
	StandardTooltip->Setup(TD.Name, TD.Value, TD.Descriptions);
}

void UUpgradesWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (StandardTooltip)
	{
		StandardTooltip->RemoveFromParent();
		StandardTooltip = nullptr;
	}

	CurrentHovered = nullptr;
}

FReply UUpgradesWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	FVector2D CursorPos = InMouseEvent.GetScreenSpacePosition();

	TArray<UWidget*> Hoverables = {
		B_HP, B_DEF, B_STR, B_AGI, B_INT, B_Efficiency, B_Level, B_LUCK, B_Regen,
		B_MetaLevel, B_MetaPoints 
	};

	UWidget* FoundHovered = nullptr;
	for (UWidget* Widget : Hoverables)
	{
		if (!Widget) continue;

		if (Widget->GetCachedGeometry().IsUnderLocation(CursorPos))
		{
			FoundHovered = Widget;
			break;
		}
	}

	if (FoundHovered != CurrentHovered)
	{
		if (StandardTooltip)
		{
			StandardTooltip->RemoveFromParent();
			StandardTooltip = nullptr;
		}
		CurrentHovered = FoundHovered;
	}

	if (CurrentHovered)
	{
		ShowTooltipForBorder(CurrentHovered, CursorPos, InGeometry);
	}

	return FReply::Handled();
}

void UUpgradesWidget::UpdateStatVisuals(UTextBlock* TB, URadialSlider* Slider, int Value, int MaxVal)
{
	if (TB)
	{
		TB->SetText(FText::AsNumber(Value));
	}

	if (Slider)
	{
		float Percent = (MaxVal > 0) ? (float)Value / (float)MaxVal : 0.0f;
		Slider->SetValue(Percent);
	}
}

void UUpgradesWidget::RefreshUI()
{
	if (!CurrentSaveData) return;
	FSaveData& SD = CurrentSaveData->SaveData;

	if (TB_Level)  TB_Level->SetText(FText::AsNumber(SD.MetaLevel));
	if (TB_Points) TB_Points->SetText(FText::AsNumber(SD.MetaPoints));

	if (MetaExperienceBar)
	{
		float Percent = (SD.MetaExperienceToLevel > 0) ? (SD.MetaExperience / SD.MetaExperienceToLevel) : 0.f;
		MetaExperienceBar->SetPercent(Percent);
	}

	if (TB_MetaExperience)
	{
		FString ExpString = FString::Printf(TEXT("%d / %d"), (int)SD.MetaExperience, (int)SD.MetaExperienceToLevel);
		TB_MetaExperience->SetText(FText::FromString(ExpString));
	}

	UpdateStatVisuals(TB_HP, HP_Slider, SD.MetaHealth, 10);
	UpdateStatVisuals(TB_DEF, DEF_Slider, SD.MetaDefence, 10);
	UpdateStatVisuals(TB_LUCK, LUCK_Slider, SD.MetaLuck, 10);
	UpdateStatVisuals(TB_STR, STR_Slider, SD.MetaStrength, 10);
	UpdateStatVisuals(TB_AGI, AGI_Slider, SD.MetaAgility, 10);
	UpdateStatVisuals(TB_INT, INT_Slider, SD.MetaIntelligence, 10);

	UpdateStatVisuals(TB_ResourceRegen, ResourceRegen_Slider, SD.MetaResourceRegen, 5);
	UpdateStatVisuals(TB_StartingLevel, StartingLevel_Slider, SD.MetaStartingLevel, 5);
	UpdateStatVisuals(TB_PotionEfficiency, PotionEfficiency_Slider, SD.MetaPotionEfficiency, 5);
}

void UUpgradesWidget::OnResetHPClicked()
{
	if (!CurrentSaveData) return;
	FSaveData& SD = CurrentSaveData->SaveData;

	SD.MetaPoints += SD.MetaHealth;
	SD.MetaHealth = 0;

	if (ResetSound)
	{
		UGameplayStatics::PlaySound2D(this, ResetSound);
	}

	RefreshUI();
}

void UUpgradesWidget::OnResetDEFClicked()
{
	if (!CurrentSaveData) return;
	FSaveData& SD = CurrentSaveData->SaveData;

	SD.MetaPoints += SD.MetaDefence;
	SD.MetaDefence = 0;

	if (ResetSound)
	{
		UGameplayStatics::PlaySound2D(this, ResetSound);
	}

	RefreshUI();
}

void UUpgradesWidget::OnResetLUCKClicked()
{
	if (!CurrentSaveData) return;
	FSaveData& SD = CurrentSaveData->SaveData;

	SD.MetaPoints += SD.MetaLuck;
	SD.MetaLuck = 0;
	
	if (ResetSound)
	{
		UGameplayStatics::PlaySound2D(this, ResetSound);
	}

	RefreshUI();
}

void UUpgradesWidget::OnResetSTRClicked()
{
	if (!CurrentSaveData) return;
	FSaveData& SD = CurrentSaveData->SaveData;

	SD.MetaPoints += SD.MetaStrength;
	SD.MetaStrength = 0;

	if (ResetSound)
	{
		UGameplayStatics::PlaySound2D(this, ResetSound);
	}

	RefreshUI();
}

void UUpgradesWidget::OnResetAGIClicked()
{
	if (!CurrentSaveData) return;
	FSaveData& SD = CurrentSaveData->SaveData;

	SD.MetaPoints += SD.MetaAgility;
	SD.MetaAgility = 0;

	if (ResetSound)
	{
		UGameplayStatics::PlaySound2D(this, ResetSound);
	}

	RefreshUI();
}

void UUpgradesWidget::OnResetINTClicked()
{
	if (!CurrentSaveData) return;
	FSaveData& SD = CurrentSaveData->SaveData;

	SD.MetaPoints += SD.MetaIntelligence;
	SD.MetaIntelligence = 0;

	if (ResetSound)
	{
		UGameplayStatics::PlaySound2D(this, ResetSound);
	}

	RefreshUI();
}

void UUpgradesWidget::OnResetRegenClicked()
{
	if (!CurrentSaveData) return;
	FSaveData& SD = CurrentSaveData->SaveData;

	SD.MetaPoints += SD.MetaResourceRegen;
	SD.MetaResourceRegen = 0;

	if (ResetSound)
	{
		UGameplayStatics::PlaySound2D(this, ResetSound);
	}

	RefreshUI();
}

void UUpgradesWidget::OnResetLevelClicked()
{
	if (!CurrentSaveData) return;
	FSaveData& SD = CurrentSaveData->SaveData;

	SD.MetaPoints += SD.MetaStartingLevel;
	SD.MetaStartingLevel = 0;

	if (ResetSound)
	{
		UGameplayStatics::PlaySound2D(this, ResetSound);
	}

	RefreshUI();
}

void UUpgradesWidget::OnResetEfficiencyClicked()
{
	if (!CurrentSaveData) return;
	FSaveData& SD = CurrentSaveData->SaveData;

	SD.MetaPoints += SD.MetaPotionEfficiency;
	SD.MetaPotionEfficiency = 0;

	if (ResetSound)
	{
		UGameplayStatics::PlaySound2D(this, ResetSound);
	}

	RefreshUI();
}

void UUpgradesWidget::OnUpgradeClicked(FString StatType)
{
	if (!CurrentSaveData || CurrentSaveData->SaveData.MetaPoints <= 0) return;

	FSaveData& SD = CurrentSaveData->SaveData;
	bool bUpgradeSuccessful = false;

	int DefaultMax = 10;
	int SpecialMax = 5;

	// Max 10
	if (StatType == "HP")
	{
		if (SD.MetaHealth < DefaultMax) { SD.MetaHealth++; bUpgradeSuccessful = true; }
	}
	else if (StatType == "DEF")
	{
		if (SD.MetaDefence < DefaultMax) { SD.MetaDefence++; bUpgradeSuccessful = true; }
	}
	else if (StatType == "LUCK")
	{
		if (SD.MetaLuck < DefaultMax) { SD.MetaLuck++; bUpgradeSuccessful = true; }
	}
	else if (StatType == "STR")
	{
		if (SD.MetaStrength < DefaultMax) { SD.MetaStrength++; bUpgradeSuccessful = true; }
	}
	else if (StatType == "AGI")
	{
		if (SD.MetaAgility < DefaultMax) { SD.MetaAgility++; bUpgradeSuccessful = true; }
	}
	else if (StatType == "INT")
	{
		if (SD.MetaIntelligence < DefaultMax) { SD.MetaIntelligence++; bUpgradeSuccessful = true; }
	}
	// Max 5
	else if (StatType == "REGEN")
	{
		if (SD.MetaResourceRegen < SpecialMax) { SD.MetaResourceRegen++; bUpgradeSuccessful = true; }
	}
	else if (StatType == "EFFICIENCY")
	{
		if (SD.MetaPotionEfficiency < SpecialMax) { SD.MetaPotionEfficiency++; bUpgradeSuccessful = true; }
	}
	if (StatType == "LEVEL")
	{
		if (SD.MetaStartingLevel < SpecialMax) { SD.MetaStartingLevel++; bUpgradeSuccessful = true; }
	}

	if (bUpgradeSuccessful)
	{
		CurrentSaveData->SaveData.MetaPoints--;

		if (UpgradeSound)
		{
			UGameplayStatics::PlaySound2D(this, UpgradeSound);
		}

		RefreshUI();
	}
}

void UUpgradesWidget::SetParentMenu(UMainMenuWidget* InParentMenu)
{
	ParentMenu = InParentMenu;
}

void UUpgradesWidget::BackButtonClicked()
{
	if (BackSound)
	{
		UGameplayStatics::PlaySound2D(this, BackSound);
	}

	if (ParentMenu)
	{
		ParentMenu->ShowModeSelectionWidget();
	}
}
