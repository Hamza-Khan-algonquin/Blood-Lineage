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
#include "PlayerHUDWorld.generated.h"

UCLASS()
class RELICRUNNERS_API UPlayerHUDWorld : public UUserWidget
{
	GENERATED_BODY()

public:

	void InitWidgetWithCharacter(class ARelicRunnersCharacter* InOwnerCharacter);

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Health;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Level;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Name;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ResourceBar;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ExperienceBar;

	UPROPERTY(meta = (BindWidget))
	class URadialSlider* ExperienceSlider;

protected:

	UPROPERTY()
	class ARelicRunnersCharacter* OwningCharacter;
};
