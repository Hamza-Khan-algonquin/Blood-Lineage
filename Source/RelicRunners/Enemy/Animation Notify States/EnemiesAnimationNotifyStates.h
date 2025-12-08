#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "EnemiesAnimationNotifyStates.generated.h"

//, meta = (DisplayName = "Tormented Soul Animation Notify State")
UCLASS(Blueprintable, meta = (DisplayName = "Tormented Soul Animation Notify State"))
class RELICRUNNERS_API UTormentedSoulAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class ATormentedSoulCharacter* OwnerCharacter;
public:
	virtual void NotifyBegin(class USkeletalMeshComponent* MeshComp, class UAnimSequenceBase* Animation, float TotalDuration) override;

	virtual void NotifyEnd(class USkeletalMeshComponent* MeshComp, class UAnimSequenceBase* Animation) override;
};