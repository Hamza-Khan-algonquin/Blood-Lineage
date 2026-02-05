#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"
#include "WarBannerAbility.generated.h"

UCLASS()
class RELICRUNNERS_API UWarBannerAbility : public UAbilityBase
{
    GENERATED_BODY()

public:
    UWarBannerAbility();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    virtual FName GetAbilityName() const override { return FName("War Banner"); }

private:
    FTimerHandle BannerDurationTimer;

    void SpawnBanner();
    void ApplyAllyBuffs(const FVector& BannerLocation);
    FVector GetBannerLocation() const;

    // War Banner properties
    UPROPERTY(EditDefaultsOnly, Category = "War Banner")
    float SpawnDistance = 200.f;

    UPROPERTY(EditDefaultsOnly, Category = "War Banner")
    float BuffAmount = 2.0f;

};
