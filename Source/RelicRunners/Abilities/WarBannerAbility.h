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
    FTimerHandle DurationTimerHandle;
    FTimerHandle BuffTickHandle;
    void SpawnBanner();
    void UpdateBuffs();
    void ApplyBuffToActor(AActor* Actor);
    void RemoveBuffFromActor(AActor* Actor);

    // War Banner properties
    UPROPERTY(EditDefaultsOnly, Category = "War Banner")
    float SpawnDistance = 200.f;

    float ElapsedTime = 0.f;
    const float BuffAmount = 20.f;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Banner")
    UStaticMesh* BannerMesh;

    TSet<TWeakObjectPtr<AActor>> BuffedActors;

    UPROPERTY()
    FVector CachedBannerLocation;
    
    UPROPERTY()
    bool bBannerActive = false;

    int PlayerLevel;
};
