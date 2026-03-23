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
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/ProgressBar.h"
#include <RelicRunners/Game/RelicRunnersGameInstance.h>
#include <Kismet/GameplayStatics.h>
#include <RelicRunners/Menu/Keybinds.h>
#include <RelicRunners/Item/ItemStats.h>
#include <Kismet/KismetMaterialLibrary.h>

void UPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
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

void UPlayerHUD::UpdateBloodSplatter(float HealthPercent, float InDeltaTime)
{
	float TargetIntensity = 0.f;
	if (HealthPercent < 1.f)
	{
		TargetIntensity = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 0.4f), FVector2D(1.f, 0.f), HealthPercent);

		float Pulse = (FMath::Sin(GetWorld()->GetTimeSeconds() * 3.0f) * 0.2f) + 0.8f;
		TargetIntensity *= Pulse;
	}

	CurrentBloodIntensity = FMath::FInterpTo(CurrentBloodIntensity, TargetIntensity, InDeltaTime, 2.0f);

	if (BloodMPC)
	{
		UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), BloodMPC, FName("BloodIntensity"), CurrentBloodIntensity);
	}
}

void UPlayerHUD::UpdateHUDHealth(float CurrentHealth, float MaxHealth)
{
	float HealthPercent = MaxHealth > 0 ? CurrentHealth / MaxHealth : 0.f;
	float PreviousPercent = HealthBar->GetPercent();

	HealthBar->SetPercent(HealthPercent);

	FString HealthString = (CurrentHealth >= MaxHealth) ?
		FString::Printf(TEXT("%d / %d"), FMath::RoundToInt(CurrentHealth), FMath::RoundToInt(MaxHealth)) :
		FString::Printf(TEXT("%d / %d"), FMath::RoundToInt(CurrentHealth), FMath::RoundToInt(MaxHealth));
	TB_Health->SetText(FText::FromString(HealthString));

	if (HealthPercent < PreviousPercent)
	{
		float DamageTaken = PreviousPercent - HealthPercent;

		//If damage is > 5% of health, or we weren't draining yet
		if (DamageTaken > 0.05f || !bShouldDrainHealthGhost)
		{
			bShouldDrainHealthGhost = false;
			GetWorld()->GetTimerManager().ClearTimer(HealthGhostTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(HealthGhostTimerHandle, this, &UPlayerHUD::StartHealthGhostDrain, 1.0f, false);
		}
	}
	else if (HealthPercent > GhostHealthPercent)
	{
		GhostHealthPercent = HealthPercent;
		HealthBar_1->SetPercent(GhostHealthPercent);
	}
}

void UPlayerHUD::UpdateAbilityManaCosts(float Mana1, float Mana2, float Mana3, float Mana4)
{
	if (TB_AbilityMana1) TB_AbilityMana1->SetText(FText::AsNumber(FMath::RoundToInt(Mana1)));
	if (TB_AbilityMana2) TB_AbilityMana2->SetText(FText::AsNumber(FMath::RoundToInt(Mana2)));
	if (TB_AbilityMana3) TB_AbilityMana3->SetText(FText::AsNumber(FMath::RoundToInt(Mana3)));
	if (TB_AbilityMana4) TB_AbilityMana4->SetText(FText::AsNumber(FMath::RoundToInt(Mana4)));
}

void UPlayerHUD::UpdateHUDResource(float CurrentResource, float MaxResource)
{
	float ResourcePercent = MaxResource > 0 ? CurrentResource / MaxResource : 0.f;
	float PreviousPercent = ResourceBar->GetPercent();

	ResourceBar->SetPercent(ResourcePercent);
	TB_Resource->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), FMath::RoundToInt(CurrentResource), FMath::RoundToInt(MaxResource))));

	if (ResourcePercent < PreviousPercent)
	{
		ShouldDrainGhost = false;
		GetWorld()->GetTimerManager().ClearTimer(GhostDelayTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(GhostDelayTimerHandle, this, &UPlayerHUD::StartGhostDrain, 1.0f, false);
	}

	if (ResourcePercent > GhostPercent)
	{
		GhostPercent = ResourcePercent;
		ResourceBar_1->SetPercent(GhostPercent);
	}
}

void UPlayerHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (ShouldDrainGhost && GhostPercent > ResourceBar->GetPercent())
	{
		GhostPercent = FMath::FInterpTo(GhostPercent, ResourceBar->GetPercent(), InDeltaTime, 3.f);
		ResourceBar_1->SetPercent(GhostPercent);

		if (FMath::IsNearlyEqual(GhostPercent, ResourceBar->GetPercent(), 0.001f))
		{
			GhostPercent = ResourceBar->GetPercent();
			ResourceBar_1->SetPercent(GhostPercent);
			ShouldDrainGhost = false;
		}
	}

	if (bShouldDrainHealthGhost && GhostHealthPercent > HealthBar->GetPercent())
	{
		GhostHealthPercent = FMath::FInterpTo(GhostHealthPercent, HealthBar->GetPercent(), InDeltaTime, 3.f);
		HealthBar_1->SetPercent(GhostHealthPercent);
	}
	else if (GhostHealthPercent < HealthBar->GetPercent())
	{
		GhostHealthPercent = HealthBar->GetPercent();
		HealthBar_1->SetPercent(GhostHealthPercent);
	}

	//UpdateBloodSplatter(HealthBar->GetPercent(), InDeltaTime);
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
	FString HealthPotKey = GetReadableKeyName(GI->Keys->GetKeyForAction(TEXT("Health Potion")));
	FString AbilityUpgradeKey = GetReadableKeyName(GI->Keys->GetKeyForAction(TEXT("Ability Selection")));

	if (TB_DamageAbility)
		TB_DamageAbility->SetText(FText::FromString(DamageKey));

	if (TB_UtilityAbility)
		TB_UtilityAbility->SetText(FText::FromString(UtilityKey));

	if (TB_DefenceAbility)
		TB_DefenceAbility->SetText(FText::FromString(DefenceKey));

	if (TB_UltimateAbility)
		TB_UltimateAbility->SetText(FText::FromString(UltimateKey));

	if (TB_HealthPotionBind)
		TB_HealthPotionBind->SetText(FText::FromString(HealthPotKey));

	if (TB_AbilitySelection)
		TB_AbilitySelection->SetText(FText::FromString(AbilityUpgradeKey));
}

void UPlayerHUD::RunCooldown(UTextBlock* CooldownText, float& RemainingTime, FTimerHandle& TimerHandle, float CooldownTime)
{
	if (!CooldownText || CooldownTime <= 0.f) return;
	if (RemainingTime > 0.f) return;

	RemainingTime = CooldownTime;
	CooldownText->SetText(
		FText::AsNumber(FMath::CeilToInt(RemainingTime))
	);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}

	TWeakObjectPtr<UTextBlock> CooldownTextWeak = CooldownText;
	float* RemainingTimePtr = &RemainingTime;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TimerHandle,
			[this, CooldownTextWeak, RemainingTimePtr]()
			{
				//Widget or HUD is gone — do nothing
				if (!CooldownTextWeak.IsValid() || !RemainingTimePtr)
				{
					return;
				}

				*RemainingTimePtr -= 1.f;

				if (*RemainingTimePtr <= 0.f)
				{
					CooldownTextWeak->SetText(FText::GetEmpty());
					return;
				}

				CooldownTextWeak->SetText(
					FText::AsNumber(FMath::CeilToInt(*RemainingTimePtr))
				);
			},
			1.0f,
			true
		);
	}
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