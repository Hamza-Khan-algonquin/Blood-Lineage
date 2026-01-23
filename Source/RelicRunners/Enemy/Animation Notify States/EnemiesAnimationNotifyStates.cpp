
#include "EnemiesAnimationNotifyStates.h"
#include "RelicRunners/Enemy/Characters/TormentedSoulCharacter.h"


void UTormentedSoulAnimNotifyState::NotifyBegin(class USkeletalMeshComponent* MeshComp, class UAnimSequenceBase* Animation, float TotalDuration)
{
	if (AActor* Owner = MeshComp->GetOwner())
	{
		OwnerCharacter = Cast<ATormentedSoulCharacter>(Owner);

		if (OwnerCharacter)
		{
			OwnerCharacter->IsAttacking = true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Tormented soul animation notify state put on animation or montage used on a object of class that is not Tormented Soul Character or one of their children"));
		}
	}
}

void UTormentedSoulAnimNotifyState::NotifyEnd(class USkeletalMeshComponent* MeshComp, class UAnimSequenceBase* Animation) 
{
	if (OwnerCharacter)
	{
		OwnerCharacter->IsAttacking = false;
	}
}
