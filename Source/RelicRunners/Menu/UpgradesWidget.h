// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveFileListData.h"
#include "Blueprint/UserWidget.h"
#include "Sound/SoundBase.h"
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

	int MaxValue = 5;
	void ShowTooltipForBorder(UWidget* HoveredWidget, const FVector2D& CursorPos, const FGeometry& ParentGeometry);
	class UWidget* CurrentHovered;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	UPROPERTY(EditDefaultsOnly, Category = "Tooltip")
	TSubclassOf<class UStandardTooltip> StandardTooltipClass;

	UPROPERTY()
	class UStandardTooltip* StandardTooltip;

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
	class UImage* IconPurity = nullptr;
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

	UPROPERTY(meta = (BindWidget))
	class UButton* B_ResetHP;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_ResetDEF;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_ResetLUCK;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_ResetSTR;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_ResetAGI;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_ResetINT;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_ResetRegen;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_ResetLevel;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_ResetEfficiency;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_MetaLevel;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_MetaPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> UpgradeSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> ResetSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> BackSound = nullptr;

	UFUNCTION()
	void OnResetHPClicked();

	UFUNCTION()
	void OnResetDEFClicked();

	UFUNCTION()
	void OnResetLUCKClicked();

	UFUNCTION()
	void OnResetSTRClicked();

	UFUNCTION()
	void OnResetAGIClicked();

	UFUNCTION()
	void OnResetINTClicked();

	UFUNCTION()
	void OnResetRegenClicked();

	UFUNCTION()
	void OnResetLevelClicked();

	UFUNCTION()
	void OnResetEfficiencyClicked();

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
