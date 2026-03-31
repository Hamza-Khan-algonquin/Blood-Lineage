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
#include "PlayerHUD.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API UPlayerHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct();

	void ToggleExpBar(bool value);

	void ToggleBossBar(bool value);

	void UpdateHUD(float CurrentHealth, float MaxHealth, float CurrentResource, float MaxResource, int CurrentLevel, float CurrentXP, float XPToLevel);

	void UpdateHUDAbilityPoints(int AbilityPoints);

	void UpdateHUDHealthPotions(int HealthPotions);

	void UpdateBloodSplatter(float HealthPercent, float InDeltaTime);

	void UpdateHUDHealth(float CurrentHealth, float MaxHealth);

	void UpdateHUDResource(float CurrentResource, float MaxResource);

	void UpdateHUDXP(int CurrentLevel, float CurrentXP, float XPToLevel);

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* ExpHUD;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* BossHUD;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_BossName;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* BossBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Health;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Resource;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Experience;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Level;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ExperienceBar;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ResourceBar;

	UPROPERTY(meta = (BindWidget))
	class UImage* I_BloodOverlay;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TB_DamageCooldown;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TB_DefenceCooldown;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TB_UtilityCooldown;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TB_UltimateCooldown;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TB_DamageAbility;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TB_DefenceAbility;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TB_UtilityAbility;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TB_UltimateAbility;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TB_AbilityPoints;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_HealthPotion;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_HealthPotionBind;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_AbilitySelection;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_AbilityMana1;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_AbilityMana2;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_AbilityMana3;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_AbilityMana4;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ResourceBar_1;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar_1;

	float GhostHealthPercent = 0.f;
	bool bShouldDrainHealthGhost = false;
	FTimerHandle HealthGhostTimerHandle;

	void StartHealthGhostDrain() { bShouldDrainHealthGhost = true; }

	FTimerHandle DamageCooldownHandle;
	FTimerHandle DefenceCooldownHandle;
	FTimerHandle UtilityCooldownHandle;
	FTimerHandle UltimateCooldownHandle;

	float GhostPercent = 0.f;

	bool ShouldDrainGhost = false;
	FTimerHandle GhostDelayTimerHandle;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void StartGhostDrain() { ShouldDrainGhost = true; }

	UPROPERTY(EditAnywhere, Category = "UI | Effects")
	class UMaterialParameterCollection* BloodMPC;

	float CurrentBloodIntensity = 0.f;
	float DamageRemainingTime = 0.f;
	float DefenceRemainingTime = 0.f;
	float UtilityRemainingTime = 0.f;
	float UltimateRemainingTime = 0.f;

	void UpdateBindText();
	void UpdateAbilityManaCosts(float Mana1, float Mana2, float Mana3, float Mana4);
	void RunCooldown(UTextBlock* CooldownText, float& RemainingTime, FTimerHandle& TimerHandle, float CooldownTime);

	UFUNCTION(BlueprintCallable)
	void StartDamageCooldown(float CooldownTime);

	UFUNCTION(BlueprintCallable)
	void StartDefenceCooldown(float CooldownTime);

	UFUNCTION(BlueprintCallable)
	void StartUtilityCooldown(float CooldownTime);

	UFUNCTION(BlueprintCallable)
	void StartUltimateCooldown(float CooldownTime);
};
