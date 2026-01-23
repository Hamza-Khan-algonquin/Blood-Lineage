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

#include "AbilitySystem/AbilityBase.h"  
#include "Abilities/Moonbeam.h"       
#include "Abilities/WarBannerAbility.h"
#include "Abilities/ImpunityAbility.h"
#include "Abilities/EarthquakeAbility.h"
#include "Abilities/BundleOfJoy.h"
#include "Abilities/VengefulDance.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"

#include "RelicRunnersCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AAbilityBase;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ARelicRunnersCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	//Functions
	ARelicRunnersCharacter();
	void Die();
	void PlayRandomAttackAnimation();
	bool HasValidWeaponEquipped(class UInventoryComponent* InvComp) const;
	void BasicAttack();
	void MoveInDirection(EAxis::Type Axis, float Value);
	void Look(const FInputActionValue& Value);
	void Interact();
	void DamageAbility();
	void DefenceAbility();
	void UtilityAbility();
	void UltimateAbility();
	virtual void GiveDamageAbilities();
	virtual void GiveDefenceAbilities();
	virtual void GiveUtilityAbilities();
	virtual void GiveUltimateAbilities();
	void HealthPotions();
	void SetReplicatedMeshByItemType(UObject* MeshAsset, const FString& ItemType);
	void PerformSwingTrace();
	void UpdatePlayerHUDWorldFacing();
	void TraceForInteractables();
	UFUNCTION(Server, Reliable)
	void Server_SpawnDroppedItemWorld(const struct FItemData& Data);
	FVector CalculateValidDropLocation(FVector DesiredLoc) const;
	void SyncEquippedVisuals(const TArray<struct FEquippedItemEntry>& EquippedEntries);
	void UpdateItemVisuals(UObject* MeshAsset, const FString& ItemType);
	UFUNCTION(BlueprintCallable)
	void UpdateItemVisuals(class UItemObject* Item);
	void OnRep_MeshUpdate(UObject* MeshAsset, const FString& ItemType);
	UFUNCTION()
	void OnRep_UpperMesh();
	UFUNCTION()
	void OnRep_LowerMesh();
	UFUNCTION()
	void OnRep_ArmsMesh();
	UFUNCTION()
	void OnRep_HelmetMesh();
	UFUNCTION()
	void OnRep_NecklaceMesh();
	UFUNCTION()
	void OnRep_MainhandMesh();
	UFUNCTION()
	void OnRep_OffhandMesh();
	UFUNCTION(Server, Reliable)
	void Server_HitEnemy(AActor* HitActor);
	UFUNCTION(Server, Reliable)
	void Server_SetReplicatedMeshByItemType(UObject* MeshAsset, const FString& ItemType);
	UFUNCTION(Server, Reliable)
	void Server_EquipItemByID(FGuid ID);
	UFUNCTION(Server, Reliable)
	void Server_UnequipItemByID(FGuid ID);

	//Getters
	FString GetMeshPathByItemType(const FString& ItemType);
	USkeletalMeshComponent* GetSkeletalMeshComponentByItemType(const FString& ItemType);
	UStaticMeshComponent* GetStaticMeshComponentByItemType(const FString& ItemType);
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

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
	UPROPERTY(ReplicatedUsing = OnRep_HelmetMesh)
	USkeletalMesh* ReplicatedHelmetMesh;
	UPROPERTY(ReplicatedUsing = OnRep_UpperMesh)
	USkeletalMesh* ReplicatedUpperMesh;
	UPROPERTY(ReplicatedUsing = OnRep_LowerMesh)
	USkeletalMesh* ReplicatedLowerMesh;
	UPROPERTY(ReplicatedUsing = OnRep_ArmsMesh)
	USkeletalMesh* ReplicatedArmsMesh;
	UPROPERTY(ReplicatedUsing = OnRep_NecklaceMesh)
	USkeletalMesh* ReplicatedNecklaceMesh;
	UPROPERTY(ReplicatedUsing = OnRep_MainhandMesh)
	UStaticMesh* ReplicatedMainhandMesh;
	UPROPERTY(ReplicatedUsing = OnRep_OffhandMesh)
	UStaticMesh* ReplicatedOffhandMesh;

	//Properties
	TSet<AActor*> AlreadyHitActors;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class UWidgetComponent* PlayerHUDWorld;
	UPROPERTY(BlueprintReadOnly, Category = "Player")
	class ARelicRunnersPlayerState* RelicPlayerState;

protected:

	//Functions
	void Tick(float DeltaTime);
	void InitializeWeaponSockets();
	void InitializeWorldHUD();
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

	//Stored Classes
	UPROPERTY(EditDefaultsOnly, Category = "ItemActor")
	TSubclassOf<class AItemActor> ItemActorClass;
};

