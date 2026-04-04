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

    void BasicAttack();


protected:
    virtual void BeginPlay() override;
    virtual void ActivateDamageAbilities() override;
    virtual void ActivateDefenceAbilities() override;
    virtual void ActivateUtilityAbilities() override;
    virtual void ActivateUltimateAbilities() override;

    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    void GiveImpunity();
    void GiveBackStab();
    void GiveVengefulDance();
    void GiveDivinePunishment();

    virtual void PerformSwingTrace() override;
    UFUNCTION(Server, Reliable)
    void Server_BasicAttack();

    void HandleBasicAttack();
    void StartAttack(int32 ComboIndex);
    void ResetCombo();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayAttack(int32 ComboIndex);

    virtual void HeavyAttack() override;

    virtual void UpdateCamera(float DeltaTime) override;
    void DetectEnemies(bool& bEnemiesNearby, AActor*& NearestEnemy, float& NearestDistSq);
    void UpdateCameraPosition(float DeltaTime, const FVector& Velocity, float Speed, bool bEnemiesNearby);
    void UpdateCameraRotation(float DeltaTime, const FVector& Velocity, float Speed);

    UPROPERTY()
    float Damage;
    bool bIsAttacking = false;
    int32 CurrentComboIndex = 0;
    bool bBufferedInput = false;
    UPROPERTY(EditAnywhere, Category = "Combat")
    float ComboResetTime = 0.5f; // time allowed between clicks

    FTimerHandle ComboResetTimer;

    UPROPERTY(EditDefaultsOnly, Category = "Nemesis")
    TSubclassOf<UBackStab> BP_BackStabClass;

    UPROPERTY(EditDefaultsOnly, Category = "Nemesis")
    TSubclassOf<UNaturesBlessing> BP_NaturesBlessingClass;

    UPROPERTY(EditDefaultsOnly, Category = "Nemesis")
    TSubclassOf<UVengefulDance> BP_VengefulDanceClass;

    UPROPERTY(EditDefaultsOnly, Category = "Nemesis")
    TSubclassOf<UDivinePunishment> BP_DivinePunishmentClass;
};
