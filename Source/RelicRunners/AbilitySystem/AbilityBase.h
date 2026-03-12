#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"  
#include "RelicRunners/PlayerController/RelicRunnersPlayerController.h"
#include "RelicRunners/PlayerHUD/PlayerHUD.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "AbilityBase.generated.h"

UENUM(BlueprintType)
enum class EAbilityCategory : uint8
{
    Offensive,
    Defensive,
    Utility,
    Ultimate, 
    None
};

UENUM(BlueprintType)
enum class EAbilityTargetType : uint8
{
    None,
    Self,
    SingleEnemy,
    Area,
    Cone
};


UCLASS(Abstract, Blueprintable)
class RELICRUNNERS_API UAbilityBase : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UAbilityBase();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

    void CalculateAbilityDamage(AActor* InstigatorActor, AActor* TargetActor, float BaseDamage);

    void CalculateAbilityMana(AActor* InstigatorActor, float BaseMana);

    UAnimMontage* PlayAbilityAnimation(const FGameplayAbilityActorInfo* ActorInfo, UAnimMontage* Montage);

    virtual FName GetAbilityName() const { return FName(TEXT("AbilityBase")); }

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Config")
    float Duration = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Combat")
    float Damage = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Combat")
    float AreaRadius = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Combat")
    float ConeAngle = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Combat")
    float LastUseTime = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Combat")
    float CooldownTimer = 0.f;


    //Ability Info
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Info")
    FName AbilityName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Info")
    EAbilityCategory AbilityCategory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Info")
    EAbilityTargetType TargetType;


    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Cooldown")
    float CooldownDuration = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Cost")
    int Cost = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
    TObjectPtr<UNiagaraSystem> AbilityVFX = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
    TObjectPtr<UNiagaraSystem> AbilityVFX1 = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* AbilityAnim;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* AbilityAnim1;

};

