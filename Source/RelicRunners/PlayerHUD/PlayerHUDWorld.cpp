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

#include "PlayerHUDWorld.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/RadialSlider.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include <RelicRunners/Game/RelicRunnersGameInstance.h>

void UPlayerHUDWorld::InitWidgetWithCharacter(ARelicRunnersCharacter* InOwnerCharacter)
{
	if (!InOwnerCharacter) return;

	OwningCharacter = InOwnerCharacter;

}

void UPlayerHUDWorld::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!OwningCharacter) return;

	int CurrentHealth = OwningCharacter->GetPlayerHealth();
	int MaxHealth = OwningCharacter->GetPlayerMaxHealth();

	if (HealthBar)
	{
		float Percent = (MaxHealth > 0) ? (float)CurrentHealth / (float)MaxHealth : 0.f;
		HealthBar->SetPercent(Percent);
	}
	if (TB_Health)
	{
		TB_Health->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentHealth, MaxHealth)));
	}

	int CurrentResource = OwningCharacter->GetPlayerResource();
	int MaxResource = OwningCharacter->GetPlayerMaxResource();
	if (ResourceBar)
	{
		float ResourcePercent = (MaxResource > 0) ? (float)CurrentResource / (float)MaxResource : 0.f;
		ResourceBar->SetPercent(ResourcePercent);
	}

	int CurrentXP = OwningCharacter->GetPlayerXP();
	int RequiredXP = OwningCharacter->GetPlayerXPToLevel();
	if (ExperienceBar)
	{
		float XPPercent = (RequiredXP > 0) ? (float)CurrentXP / (float)RequiredXP : 0.f;
		//ExperienceBar->SetPercent(XPPercent);
		ExperienceSlider->SetValue(XPPercent);
	}

	if (TB_Name)
	{
		if (ARelicRunnersPlayerState* PS = Cast<ARelicRunnersPlayerState>(OwningCharacter->GetPlayerState()))
		{
			TB_Name->SetText(FText::FromString(FString::Printf(TEXT("%s"), *PS->GetPlayerName())));
		}
	}

	if (TB_Level)
	{
		int Level = OwningCharacter->GetPlayerLevel();
		TB_Level->SetText(FText::FromString(FString::Printf(TEXT("%d"), Level)));
	}
}