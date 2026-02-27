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

#include "AnimationNotifyState.h"
#include <RelicRunners/RelicRunnersCharacter.h>

void UAnimationNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (ARelicRunnersCharacter* Character = Cast<ARelicRunnersCharacter>(Owner))
		{
			Character->PerformSwingTrace(); 
		}
	}
}