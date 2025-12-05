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

#include "LevelUpHUD.h"
#include "Components/TextBlock.h"
#include "RelicRunners/RelicRunnersCharacter.h"


void ULevelUpHUD::NativeConstruct()
{
	Super::NativeConstruct();

	if (Fade)
	{
		PlayAnimation(Fade);
	}

	//Auto-remove after 4 seconds
	FTimerHandle RemoveHandle;
	GetWorld()->GetTimerManager().SetTimer(RemoveHandle, this, &ULevelUpHUD::RemoveFromParent, 4.0f, false);

	EvaluatePlayerLevel();
}

void ULevelUpHUD::EvaluatePlayerLevel()
{
	APlayerController* PC = GetOwningPlayer();
	PC = GetOwningPlayer();
	if (PC)
	{
		APawn* Pawn = PC->GetPawn();
		OwningCharacter = Cast<ARelicRunnersCharacter>(Pawn);

		TB_Level->SetText(FText::FromString(FString::Printf(TEXT(" %d"), OwningCharacter->GetPlayerLevel())));
	}
	
}


