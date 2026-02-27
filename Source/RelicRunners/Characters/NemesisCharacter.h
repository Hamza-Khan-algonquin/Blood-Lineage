#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/RelicRunnersCharacter.h" 
#include "NemesisCharacter.generated.h"

/**
 *
 */
//class USpringArmComponent;
//class UCameraComponent;

UCLASS()
class RELICRUNNERS_API ANemesisCharacter : public ARelicRunnersCharacter
{
    GENERATED_BODY()

public:
    ANemesisCharacter();

    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    class UClassInfo* ClassInfo;

    virtual void Server_HitEnemy_Implementation(AActor* HitActor) override;

protected:
    virtual void BeginPlay() override;
    virtual void ActivateDamageAbilities() override;
    virtual void ActivateDefenceAbilities() override;
    virtual void ActivateUtilityAbilities() override;
    virtual void ActivateUltimateAbilities() override;

    void GiveImpunity();
    void GiveBackStab();
    void GiveVengefulDance();
    void GiveDivinePunishment();

    virtual void PlayRandomAttackAnimation() override;
    virtual void PerformSwingTrace() override;
    virtual void BasicAttack() override;
    virtual void HeavyAttack() override;
    virtual void UpdateCamera(float DeltaTime) override;

    UPROPERTY()
    float Damage;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* BasicAnim;

};
