#include "UpgradesWidget.h"
#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Textblock.h"
#include "Components/ProgressBar.h"
#include "Components/RadialSlider.h"
#include "RelicRunners/Item/ItemStats.h"
#include "RelicRunners/Menu/SaveFileListData.h"
#include "RelicRunners/Menu/SaveFileListWidget.h"

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
	if (IconRegen)	IconRegen->SetColorAndOpacity(ItemStats::GetIconColor("Resource"));
	if (IconLevel)	IconLevel->SetColorAndOpacity(ItemStats::GetIconColor("Level"));

	if (B_HP)			B_HP->OnClicked.AddDynamic(this, &UUpgradesWidget::OnHPClicked);
	if (B_DEF)			B_DEF->OnClicked.AddDynamic(this, &UUpgradesWidget::OnDEFClicked);
	if (B_LUCK)			B_LUCK->OnClicked.AddDynamic(this, &UUpgradesWidget::OnLUCKClicked);
	if (B_STR)			B_STR->OnClicked.AddDynamic(this, &UUpgradesWidget::OnSTRClicked);
	if (B_AGI)			B_AGI->OnClicked.AddDynamic(this, &UUpgradesWidget::OnAGIClicked);
	if (B_INT)			B_INT->OnClicked.AddDynamic(this, &UUpgradesWidget::OnINTClicked);
	if (B_Regen)		B_Regen->OnClicked.AddDynamic(this, &UUpgradesWidget::OnRegenClicked);
	if (B_Level)		B_Level->OnClicked.AddDynamic(this, &UUpgradesWidget::OnLevelClicked);
	if (B_Efficiency)	B_Efficiency->OnClicked.AddDynamic(this, &UUpgradesWidget::OnEfficiencyClicked);
}

void UUpgradesWidget::InitializeUpgradeData(USaveFileListData* InSaveData)
{
	CurrentSaveData = InSaveData;
	RefreshUI();
}

void UUpgradesWidget::UpdateStatVisuals(UTextBlock* TB, URadialSlider* Slider, int Value)
{
	float MaxVal = 5.0f;

	if (TB)
	{
		TB->SetText(FText::AsNumber(Value));
	}

	if (Slider)
	{
		Slider->SetValue((float)(Value) / MaxVal);
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

	UpdateStatVisuals(TB_HP, HP_Slider, SD.MetaHealth);
	UpdateStatVisuals(TB_DEF, DEF_Slider, SD.MetaDefence);
	UpdateStatVisuals(TB_LUCK, LUCK_Slider, SD.MetaLuck);
	UpdateStatVisuals(TB_STR, STR_Slider, SD.MetaStrength);
	UpdateStatVisuals(TB_AGI, AGI_Slider, SD.MetaAgility);
	UpdateStatVisuals(TB_INT, INT_Slider, SD.MetaIntelligence);
	UpdateStatVisuals(TB_ResourceRegen, ResourceRegen_Slider, SD.MetaResourceRegen);
	UpdateStatVisuals(TB_StartingLevel, StartingLevel_Slider, SD.MetaStartingLevel);
	UpdateStatVisuals(TB_PotionEfficiency, PotionEfficiency_Slider, SD.MetaPotionEfficiency);
}

void UUpgradesWidget::OnUpgradeClicked(FString StatType)
{
	if (!CurrentSaveData || CurrentSaveData->SaveData.MetaPoints <= 0) return;

	CurrentSaveData->SaveData.MetaPoints--;

	FSaveData& SD = CurrentSaveData->SaveData;

	if (StatType == "HP") { SD.MetaHealth++; }
	else if (StatType == "DEF") { SD.MetaDefence++; }
	else if (StatType == "LUCK") { SD.MetaLuck++; }
	else if (StatType == "STR") { SD.MetaStrength++; }
	else if (StatType == "AGI") { SD.MetaAgility++; }
	else if (StatType == "INT") { SD.MetaIntelligence++; }
	else if (StatType == "REGEN") { SD.MetaResourceRegen++; }
	else if (StatType == "LEVEL") { SD.MetaStartingLevel++; }
	else if (StatType == "EFFICIENCY") { SD.MetaPotionEfficiency++; }

	//Update the visuals
	RefreshUI();
}

void UUpgradesWidget::SetParentMenu(UMainMenuWidget* InParentMenu)
{
	ParentMenu = InParentMenu;
}

void UUpgradesWidget::BackButtonClicked()
{
	if (ParentMenu)
	{
		ParentMenu->ShowModeSelectionWidget();
	}
}