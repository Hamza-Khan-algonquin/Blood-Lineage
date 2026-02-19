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

#include "PlayerHUD.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/ProgressBar.h"
#include <RelicRunners/Game/RelicRunnersGameInstance.h>
#include <Kismet/GameplayStatics.h>
#include <RelicRunners/Menu/Keybinds.h>

void UPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateBindText();
}

void UPlayerHUD::ToggleExpBar(bool value)
{
	ExpHUD->SetVisibility(value ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UPlayerHUD::ToggleBossBar(bool value)
{
	BossHUD->SetVisibility(value ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UPlayerHUD::UpdateHUD(float CurrentHealth, float MaxHealth, float CurrentResource, float MaxResource, int CurrentLevel, float CurrentXP, float XPToLevel)
{
	if (!HealthBar || !ResourceBar || !ExperienceBar || !TB_Health || !TB_Resource || !TB_Level || !TB_Experience)
	{
		UE_LOG(LogTemp, Warning, TEXT("HUD component not initialized properly"));
		return;
	}

	float HealthPercent = MaxHealth > 0 ? CurrentHealth / MaxHealth : 0.f;
	float ResourcePercent = MaxResource > 0 ? CurrentResource / MaxResource : 0.f;
	float ExperiencePercent = XPToLevel > 0 ? CurrentXP / XPToLevel : 0.f;

	HealthBar->SetPercent(HealthPercent);
	ResourceBar->SetPercent(ResourcePercent);
	ExperienceBar->SetPercent(ExperiencePercent);

	TB_Health->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), FMath::RoundToInt(CurrentHealth), FMath::RoundToInt(MaxHealth))));
	TB_Resource->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), FMath::RoundToInt(CurrentResource), FMath::RoundToInt(MaxResource))));
	TB_Level->SetText(FText::FromString(FString::Printf(TEXT("%d"), CurrentLevel)));
	TB_Experience->SetText(FText::FromString(FString::Printf(TEXT("%d / %d XP"), FMath::RoundToInt(CurrentXP), FMath::RoundToInt(XPToLevel))));
}

void UPlayerHUD::UpdateHUDAbilityPoints(int AbilityPoints)
{
	TB_AbilityPoints->SetText(FText::FromString(FString::Printf(TEXT("%d"), AbilityPoints)));
}

void UPlayerHUD::UpdateHUDHealthPotions(int HealthPotions)
{
	TB_HealthPotion->SetText(FText::FromString(FString::Printf(TEXT("%d"), HealthPotions)));
}

void UPlayerHUD::UpdateHUDHealth(float CurrentHealth, float MaxHealth)
{
	float HealthPercent = MaxHealth > 0 ? CurrentHealth / MaxHealth : 0.f;

	HealthBar->SetPercent(HealthPercent);
	TB_Health->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), FMath::RoundToInt(CurrentHealth), FMath::RoundToInt(MaxHealth))));
}

void UPlayerHUD::UpdateHUDResource(float CurrentResource, float MaxResource)
{
	float ResourcePercent = MaxResource > 0 ? CurrentResource / MaxResource : 0.f;

	ResourceBar->SetPercent(ResourcePercent);
	TB_Resource->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), FMath::RoundToInt(CurrentResource), FMath::RoundToInt(MaxResource))));
}

void UPlayerHUD::UpdateHUDXP(int CurrentLevel, float CurrentXP, float XPToLevel)
{
	float ExperiencePercent = XPToLevel > 0 ? CurrentXP / XPToLevel : 0.f;

	ExperienceBar->SetPercent(ExperiencePercent);
	TB_Level->SetText(FText::FromString(FString::Printf(TEXT("%d"), CurrentLevel)));
	TB_Experience->SetText(FText::FromString(FString::Printf(TEXT("%d / %d XP"), FMath::RoundToInt(CurrentXP), FMath::RoundToInt(XPToLevel))));
}


void UPlayerHUD::UpdateBindText()
{
	UWorld* World = GetWorld();
	if (!World) return;

	URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(UGameplayStatics::GetGameInstance(World));
	if (!GI || !GI->Keys) return;

	FString DamageKey = GetReadableKeyName(GI->Keys->GetKeyForAction(TEXT("Damage Ability")));
	FString UtilityKey = GetReadableKeyName(GI->Keys->GetKeyForAction(TEXT("Utility Ability")));
	FString DefenceKey = GetReadableKeyName(GI->Keys->GetKeyForAction(TEXT("Defence Ability")));
	FString UltimateKey = GetReadableKeyName(GI->Keys->GetKeyForAction(TEXT("Ultimate Ability")));

	if (TB_DamageAbility)
		TB_DamageAbility->SetText(FText::FromString(DamageKey));

	if (TB_UtilityAbility)
		TB_UtilityAbility->SetText(FText::FromString(UtilityKey));

	if (TB_DefenceAbility)
		TB_DefenceAbility->SetText(FText::FromString(DefenceKey));

	if (TB_UltimateAbility)
		TB_UltimateAbility->SetText(FText::FromString(UltimateKey));
}

void UPlayerHUD::RunCooldown(UTextBlock* CooldownText, float& RemainingTime, FTimerHandle& TimerHandle, float CooldownTime)
{
	if (!CooldownText || CooldownTime <= 0.f) return;

	if (RemainingTime > 0.f)
		return;

	RemainingTime = CooldownTime;
	CooldownText->SetText(FText::AsNumber(FMath::CeilToInt(RemainingTime)));

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, CooldownText, &RemainingTime, &TimerHandle]()
		{
			RemainingTime -= 1.0f;
			if (!CooldownText) GetWorld()->GetTimerManager().ClearTimer(TimerHandle); return;

			if (RemainingTime <= 0.f)
			{
				CooldownText->SetText(FText::FromString(TEXT("")));
				GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
			}
			else
			{
				CooldownText->SetText(FText::AsNumber(FMath::CeilToInt(RemainingTime)));
			}
		}, 1.0f, true
	);
}
void UPlayerHUD::StartDamageCooldown(float CooldownTime)
{
	RunCooldown(TB_DamageCooldown, DamageRemainingTime, DamageCooldownHandle, CooldownTime);
}

void UPlayerHUD::StartDefenceCooldown(float CooldownTime)
{
	RunCooldown(TB_DefenceCooldown, DefenceRemainingTime, DefenceCooldownHandle, CooldownTime);
}

void UPlayerHUD::StartUtilityCooldown(float CooldownTime)
{
	RunCooldown(TB_UtilityCooldown, UtilityRemainingTime, UtilityCooldownHandle, CooldownTime);
}

void UPlayerHUD::StartUltimateCooldown(float CooldownTime)
{
	RunCooldown(TB_UltimateCooldown, UltimateRemainingTime, UltimateCooldownHandle, CooldownTime);
}