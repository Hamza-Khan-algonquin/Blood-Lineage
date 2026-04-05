// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Sound/SoundBase.h"
#include "AbilitySelection.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API UAbilitySelection : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_Upgrade1;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_Upgrade2;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_Upgrade3;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_Upgrade4;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Upgrade1;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Upgrade2;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Upgrade3;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Upgrade4;

	UPROPERTY(meta = (BindWidget))
	class UImage* Icon_0;

	UPROPERTY(meta = (BindWidget))
	class UImage* Icon_1;

	UPROPERTY(meta = (BindWidget))
	class UImage* Icon_2;

	UPROPERTY(meta = (BindWidget))
	class UImage* Icon_3;

	UPROPERTY()
	class ARelicRunnersCharacter* OwningCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> UpgradePickSound = nullptr;

	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	void ShowTooltipForWidget(UWidget* HoveredWidget, const FVector2D& CursorPos, const FGeometry& ParentGeometry);
	class UWidget* CurrentHoveredWidget;
	UPROPERTY()
	class UStandardTooltip* StandardTooltip;
	UPROPERTY(EditDefaultsOnly, Category = "Tooltip")
	TSubclassOf<class UStandardTooltip> StandardTooltipClass;
	UFUNCTION()
	void Upgrade1Clicked();

	UFUNCTION()
	void Upgrade2Clicked();

	UFUNCTION()
	void Upgrade3Clicked();

	UFUNCTION()
	void Upgrade4Clicked();

	enum class EStats : uint8
	{
		Health         UMETA(DisplayName = "Health"),
		Armor          UMETA(DisplayName = "Armor"),
		Dexterity      UMETA(DisplayName = "Dexterity"),
		Strength       UMETA(DisplayName = "Strength"),
		Intelligence   UMETA(DisplayName = "Intelligence"),
	};

	struct StatBuff
	{
		int Percentage;
		EStats StatType;
		FString StatText;
		UTexture2D* Icon;
		FLinearColor Color;
	};

	StatBuff RandomStatBuff(const TArray<EStats>& ExcludedStats);
	void RefreshUpgrades();
	void BuffStatValue(StatBuff Buff);

	enum class EAbilityUpgrades : uint8
	{
		Health1         UMETA(DisplayName = "Health"),
		Armor1          UMETA(DisplayName = "Armor"),
		Dexterity1     UMETA(DisplayName = "Dexterity"),
		Strength1       UMETA(DisplayName = "Strength"),
		Intelligence1   UMETA(DisplayName = "Intelligence"),
	};


	struct AbilityUpgrade
	{
		FString AbilityType;
		FString AbilityUpgrade;
		class UImage* AbilityImage;
		FString AbilityText;
	};

	StatBuff Buff1;
	StatBuff Buff2;
	StatBuff Buff3;
	StatBuff Buff4;
};
