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
#include "Blueprint/UserWidget.h"
#include "InventoryToolTip.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API UInventoryToolTip : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	//Functions
	void Setup(struct FItemData& ItemData, const FLinearColor& RarityColor);
	void Setup(class UItemObject* HoveredItem, const FLinearColor& HoveredColor, class UItemObject* EquippedItem);

	//UI Elements
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* ItemTooltip;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_LevelTooltip;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_RarityTooltip;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_ItemTypeTooltip;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_HealthTooltip;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_ArmorTooltip;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_DexterityTooltip;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_StrengthTooltip;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_IntelligenceTooltip;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_HealthValue;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_ArmorValue;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_DexterityValue;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_StrengthValue;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_IntelligenceValue;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_HealthValue_2;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_ArmorValue_2;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_DexterityValue_2;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_StrengthValue_2;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_IntelligenceValue_2;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_GoldTooltip;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_FromWhere;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_Rarity;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_ItemType;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_Health;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_Armor;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_Dexterity;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_Strength;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_Intelligence;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_Gold;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_FromWhere;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* HB_Health;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* HB_Armor;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* HB_Dexterity;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* HB_Strength;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* HB_Intelligence;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* HB_Gold;

	protected:

		UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UHorizontalBox* HB_Luck;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UBorder* B_Luck;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* TB_LuckTooltip; 

		UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* TB_LuckValue; 

		UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* TB_LuckValue_2; 

		UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UHorizontalBox* HB_Regen;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UBorder* B_Regen;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* TB_RegenTooltip;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* TB_RegenValue;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* TB_RegenValue_2;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UHorizontalBox* HB_Purity;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UBorder* B_Purity;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* TB_PurityTooltip;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* TB_PurityValue;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* TB_PurityValue_2;
};
