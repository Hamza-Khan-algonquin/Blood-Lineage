// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"
#include "PiercingArrow.generated.h"

UCLASS()
class RELICRUNNERS_API UPiercingArrow : public UAbilityBase
{
    GENERATED_BODY()

public:
    UPiercingArrow();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

    virtual FName GetAbilityName() const override { return FName("Piercing Arrow"); }

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PiercingArrow")
    float ArrowSpeed = 2200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PiercingArrow")
    float MaxRange = 3000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PiercingArrow")
    float ArrowRadius = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PiercingArrow")
    float GrowthRate = 15.f; // this is what is supposed to make the arrow grow bigger...

private:
    FVector ArrowLocation;
    FVector ArrowDirection;
    float TraveledDistance = 0.f;

    bool bIsFlying = false;

    FTimerHandle ArrowTickTimer;

    TSet<AActor*> HitActors;

    void TickArrow();
};