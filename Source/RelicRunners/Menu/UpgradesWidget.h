// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UpgradesWidget.generated.h"

UCLASS()
class RELICRUNNERS_API UUpgradesWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	void SetParentMenu(class UMainMenuWidget* InParentMenu);

	UFUNCTION(BlueprintCallable)
	void InitializeUpgradeData(class USaveFileListData* InSaveData);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* BackButton = nullptr;

	UPROPERTY()
	class UMainMenuWidget* ParentMenu;

	UPROPERTY()
	class USaveFileListData* CurrentSaveData = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UProgressBar* MetaExperienceBar = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_MetaExperience = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_Points = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_Level = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* IconHP = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* IconDEF = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* IconLUCK = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* IconSTR = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* IconAGI = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* IconINT = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* IconRegen = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* IconLevel = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* B_HP = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* B_DEF = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* B_LUCK = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* B_STR = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* B_AGI = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* B_INT = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* B_Regen = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* B_Level = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* B_Efficiency = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class URadialSlider* HP_Slider = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class URadialSlider* DEF_Slider = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class URadialSlider* LUCK_Slider = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class URadialSlider* STR_Slider = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class URadialSlider* AGI_Slider = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class URadialSlider* INT_Slider = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class URadialSlider* ResourceRegen_Slider = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class URadialSlider* StartingLevel_Slider = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class URadialSlider* PotionEfficiency_Slider = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_HP = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_DEF = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_LUCK = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_STR = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_AGI = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_INT = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_ResourceRegen = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_StartingLevel = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_PotionEfficiency = nullptr;

	UFUNCTION()
	void BackButtonClicked();

	void UpdateStatVisuals(class UTextBlock* TB, class URadialSlider* Slider, int Value);

	void RefreshUI();

	UFUNCTION()
	void OnUpgradeClicked(FString StatType);
	UFUNCTION() 
	void OnHPClicked() { OnUpgradeClicked("HP"); }
	UFUNCTION() 
	void OnDEFClicked() { OnUpgradeClicked("DEF"); }
	UFUNCTION() 
	void OnLUCKClicked() { OnUpgradeClicked("LUCK"); }
	UFUNCTION() 
	void OnSTRClicked() { OnUpgradeClicked("STR"); }
	UFUNCTION() 
	void OnAGIClicked() { OnUpgradeClicked("AGI"); }
	UFUNCTION() 
	void OnINTClicked() { OnUpgradeClicked("INT"); }
	UFUNCTION() 
	void OnRegenClicked() { OnUpgradeClicked("REGEN"); }
	UFUNCTION() 
	void OnLevelClicked() { OnUpgradeClicked("LEVEL"); }
	UFUNCTION() 
	void OnEfficiencyClicked() { OnUpgradeClicked("EFFICIENCY"); }
};
