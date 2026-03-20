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

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Abilities/PiercingArrow.h"
#include "AbilitySystem/AbilityBase.h"  
#include "Abilities/Moonbeam.h"       
#include "Abilities/GoddessOfLove.h"       
#include "Abilities/WarBannerAbility.h"
#include "Abilities/ImpunityAbility.h"
#include "Abilities/EarthquakeAbility.h"
#include "RelicRunners/Abilities/GuardAbility.h"
#include "Abilities/BackStab.h"
#include "Abilities/BundleOfJoy.h"
#include "Abilities/MarkOfTheHunt.h"
#include "Abilities/RageOfTheGods.h"
#include "Abilities/VengefulDance.h"
#include "Abilities/DivinePunishment.h"
#include "Abilities/NatureBlessing.h"
#include "RelicRunners/Abilities/MirrorOfDesireAbility.h"
#include "Abilities/KissOfLife.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "TimerManager.h"
#include "Components/SphereComponent.h"
#include "RelicRunnersCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AAbilityBase;
struct FInputActionValue;

USTRUCT(BlueprintType)
struct FEquippedVisuals
{
	GENERATED_BODY()

	UPROPERTY()
	UStaticMesh* Mainhand = nullptr;

	UPROPERTY()
	UStaticMesh* Offhand = nullptr;

	UPROPERTY()
	USkeletalMesh* Upper = nullptr;

	UPROPERTY()
	USkeletalMesh* Lower = nullptr;

	UPROPERTY()
	USkeletalMesh* Helmet = nullptr;

	UPROPERTY()
	USkeletalMesh* Arms = nullptr;

	UPROPERTY()
	USkeletalMesh* Necklace = nullptr;
};

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ARelicRunnersCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	//void OnPause();

	//void ContinueMontage();

	//Functions
	ARelicRunnersCharacter();
	void Die();
	virtual void PlayRandomAttackAnimation();
	bool HasValidWeaponEquipped(class UInventoryComponent* InvComp) const;
	virtual void BasicAttack();
	virtual void HeavyAttack();
	virtual void ProjectileHeavyAttack();
	void MoveInDirection(EAxis::Type Axis, float Value);
	void Look(const FInputActionValue& Value);
	void Interact();
	void DamageAbility();
	void DefenceAbility();
	void UtilityAbility();
	void UltimateAbility();
	virtual void ActivateDamageAbilities();
	virtual void ActivateDefenceAbilities();
	virtual void ActivateUtilityAbilities();
	virtual void ActivateUltimateAbilities();
	void HealthPotions();
	virtual void PerformSwingTrace();
	void UpdatePlayerHUDWorldFacing();
	void TraceForInteractables();
	void SetMovementSpeed(float value);
	UFUNCTION()
    void ResetBasicAttack();
	UFUNCTION()
	void ResetHeavyAttack();
	UFUNCTION(Server, Reliable)
	void Server_SpawnDroppedItemWorld(const struct FItemData& Data);
	FVector CalculateValidDropLocation(FVector DesiredLoc) const;
	void SyncEquippedVisuals(const TArray<struct FEquippedItemEntry>& EquippedEntries);
	void UpdateItemVisuals(UObject* MeshAsset, const FString& ItemType);
	UFUNCTION(BlueprintCallable)
	void UpdateItemVisuals(class UItemObject* Item);
	UFUNCTION(Server, Reliable)
	void Server_HitEnemy(AActor* TargetActor, float BaseDamage);



	//Getters
	FString GetMeshPathByItemType(const FString& ItemType);
	USkeletalMeshComponent* GetSkeletalMeshComponentByItemType(const FString& ItemType);
	UStaticMeshComponent* GetStaticMeshComponentByItemType(const FString& ItemType);
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const { return AbilitySystem; }

	//Animations
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimSequence* SwordBasicAttack0;
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimSequence* SwordBasicAttack1;
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimSequence* SwordBasicAttack2;
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimSequence* SwordBasicAttack3;

	//Meshes
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* LowerMesh;
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* UpperMesh;
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* HelmetMesh;
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* ArmsMesh;
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* NecklaceMesh;
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MainhandItemMesh;
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* OffhandItemMesh;

	UPROPERTY(ReplicatedUsing = OnRep_VisualsChanged)
	FEquippedVisuals ReplicatedVisuals;
	UFUNCTION()
	void OnRep_VisualsChanged();

	//Properties
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool CanBasicAttack = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BasicAttackCooldown = 0.5f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool CanHeavyAttack = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float HeavyAttackCooldown = 2.0f;
	TSet<AActor*> AlreadyHitActors;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class UWidgetComponent* PlayerHUDWorld;
	UPROPERTY(BlueprintReadOnly, Category = "Player")
	class ARelicRunnersPlayerState* RelicPlayerState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Minimap")
	class USceneCaptureComponent2D* MinimapCaptureComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Minimap")
	USpringArmComponent* MinimapSpringArm;
	

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystem;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* BasicAnim;

	//Functions
	void Tick(float DeltaTime);
	virtual void UpdateCamera(float DeltaTime);
	void InitializeWorldHUD();
	void InitializeWeaponSockets();
	void RegisterDirector();

	//Virtual Functions
	virtual void BeginPlay();
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;


	//Properties
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector respawnPoint;

	//Timers
	FTimerHandle HealthRegenTimerHandle;
	FTimerHandle DelegateBindHandle;
	FTimerHandle TimerHandle_AttackCooldown;
	FTimerHandle TimerHandle_HeavyAttackCooldown;

	//Stored Classes
	UPROPERTY(EditDefaultsOnly, Category = "ItemActor")
	TSubclassOf<class AItemActor> ItemActorClass;


	// Camera smoothing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Smooth")
	float CameraYawSmoothSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Smooth")
	float CameraPitchSmoothSpeed = 10.f;

	// Camera look-ahead offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Smooth")
	float LookAheadDistance = 50.f;

	// Height & offset tweaks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Position")
	float HeightOffset = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Position")
	float ShoulderOffset = 40.f;

	// Arm length settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float BaseArmLength = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float SprintArmLength = 450.f;

	// Pitch limits
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Limits")
	float MinPitch = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Limits")
	float MaxPitch = 25.f;

	// Optional bounce
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Bounce")
	float BounceAmplitude = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Bounce")
	float BounceSpeed = 6.f;


	FVector CurrentLookAhead;

};

