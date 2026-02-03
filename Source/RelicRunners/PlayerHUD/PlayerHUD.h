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

	void NativeConstruct();

	void UpdateHUD(float CurrentHealth, float MaxHealth, float CurrentResource, float MaxResource, int CurrentLevel, float CurrentXP, float XPToLevel);

	void UpdateHUDAbilityPoints(int AbilityPoints);

	void UpdateHUDHealthPotions(int HealthPotions);

	void UpdateHUDHealth(float CurrentHealth, float MaxHealth);

	void UpdateHUDResource(float CurrentResource, float MaxResource);

	void UpdateHUDXP(int CurrentLevel, float CurrentXP, float XPToLevel);

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

	FTimerHandle DamageCooldownHandle;
	FTimerHandle DefenceCooldownHandle;
	FTimerHandle UtilityCooldownHandle;
	FTimerHandle UltimateCooldownHandle;

	float DamageRemainingTime = 0.f;
	float DefenceRemainingTime = 0.f;
	float UtilityRemainingTime = 0.f;
	float UltimateRemainingTime = 0.f;

	void UpdateBindText();

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
