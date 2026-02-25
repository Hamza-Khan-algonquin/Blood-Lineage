#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"
#include "BundleOfJoy.generated.h"

class AEnemyPawn;

UCLASS()
class RELICRUNNERS_API UBundleOfJoy : public UAbilityBase
{
    GENERATED_BODY()

public:
    UBundleOfJoy();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    void TickThrown();
    void OnThrownLanded(const FVector& LandLocation);
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    virtual FName GetAbilityName() const override { return FName("Bundle Of Joy"); }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BundleOfJoy")
    float AttractionRadius = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BundleOfJoy")
    float ExplosionRadius = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BundleOfJoy")
    float AttractionStrength = 800.f;



private:
    FTimerHandle ExplosionTimer;
    FTimerHandle AttractTimer;

    FVector StartLocation;
    FVector Velocity;
    bool bIsFlying = false;

    UPROPERTY()
    FVector AttractionCenter;

    TArray<AEnemyPawn*> EffectedEnemies;

    void Explode();
    void Attract();
};
