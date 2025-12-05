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

enum class EInventorySorting : uint8;

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

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

public:
	ARelicRunnersCharacter();

	//Replication
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	EInventorySorting GetCurrentSortingMethod() const;
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

	//Server
	UFUNCTION(Server, Reliable)
	void Server_SetReplicatedMeshByItemType(UObject* MeshAsset, const FString& ItemType);
	UFUNCTION(Server, Reliable)
	void Server_EquipItemByID(FGuid ID);
	UFUNCTION(Server, Reliable)
	void Server_UnequipItemByID(FGuid ID);
	UFUNCTION(Server, Reliable)
	void Server_SetMaxHealth(int health);
	UFUNCTION(Server, Reliable)
	void Server_DropItem(const struct FItemData& ItemData);
	UFUNCTION(Server, Reliable)
	void Server_PickupItem(AItemActor* TargetItem);
	
	//Client
	UFUNCTION(Client, Reliable)
	void Client_UpdateEquippedStats(const FEquippedStatsSummary& NewStats);

	TSet<AActor*> AlreadyHitActors;
	
	void PerformSwingTrace();

	UFUNCTION(Server, Reliable)
	void Server_HitEnemy(AActor* HitActor);

	//Inputs
	void BasicAttack();
	void MoveInDirection(EAxis::Type Axis, float Value);
	void Look(const FInputActionValue& Value);
	void Interact();
	void ToggleUI(UUserWidget* UIWidget);
	void InventoryUI();
	void PauseUI();
	void AbilitySystemUI();
	void HideUI(UUserWidget* UIWidget, APlayerController* PlayerController, bool bClosePopups);
	void RemoveOtherUI(FString UI, APlayerController* playerController);
	void DamageAbility();
	void DefenceAbility();
	void UtilityAbility();
	void UltimateAbility();
	void HealthPotions();

	UFUNCTION(Server, Reliable)
	void Server_SpendAbilityPoints();

	UFUNCTION(Client, Reliable)
	void Client_OnAbilityPointsUpdated(int32 NewAbilityPoints);

	UFUNCTION(Server, Reliable)
	void Server_UseHealthPotion(int NewHealth, int NewPotionCount);

	//Ticking
	void UpdatePlayerHUDWorldFacing();
	void TraceForInteractables();

	//Getters
	int GetPlayerLevel() { return PlayerLevel; };
	int GetPlayerHealth() { return PlayerHealth; }
	int GetPlayerMaxHealth() { return PlayerMaxHealth; }
	int GetPlayerResource() { return PlayerResource; }
	int GetPlayerMaxResource() { return PlayerMaxResource; }
	int GetPlayerXP() { return PlayerXP; }
	int GetPlayerXPToLevel() { return PlayerXPToLevel; }
	int GetPlayerAbilityPoints() { return PlayerAbilityPoints; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetPlayerStartingArmor(int StartingArmor) { PlayerStartingArmor =  StartingArmor; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetPlayerStartingDexterity(int StartingDexterity) { PlayerStartingDexterity = StartingDexterity; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetPlayerStartingIntelligence(int StartingIntelligence) { PlayerStartingIntelligence = StartingIntelligence; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetPlayerStartingHealth(int StartingHealth) { PlayerStartingMaxHealth = StartingHealth; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetPlayerStartingStrength(int StartingStrength) { PlayerStartingStrength = StartingStrength; }

	UFUNCTION()
	int GetPlayerStartingMaxHealth() const { return PlayerStartingMaxHealth; }

	UFUNCTION()
	int GetPlayerNumInventorySlots() const { return PlayerNumInventorySlots; }

	UFUNCTION()
	int GetPlayerStartingArmor() const { return PlayerStartingArmor; }

	UFUNCTION()
	int GetPlayerStartingDexterity() const { return PlayerStartingDexterity; }

	UFUNCTION()
	int GetPlayerStartingStrength() const { return PlayerStartingStrength; }

	UFUNCTION()
	int GetPlayerStartingIntelligence() const { return PlayerStartingIntelligence; }

	UFUNCTION()
	int GetPlayerGold() const { return PlayerGold; }

	UFUNCTION()
	void AddGold(int Value);

	UFUNCTION(Server, Reliable)
	void Server_AddGold(int Value);

	UFUNCTION()
	bool CheckEnoughGold(int Value);

	UFUNCTION(Server, Reliable)
	void Server_BuyItem(const FItemData& ItemData, class AVendor* Vendor);

	UFUNCTION(Server, Reliable)
	void Server_SellItem(const FItemData& ItemData, class AVendor* Vendor);

	UFUNCTION(Client, Reliable)
	void Client_UpdateVendorUI(class AVendor* Vendor);

	class UInventory* GetInventory() { return Inventory; };

	class UInventoryComponent* GetInventoryComponent() { return InventoryComponent; };
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void SetInventoryComponent(UInventoryComponent* InvComp);

	//Update functions
	void UpdateItemVisuals(UObject* MeshAsset, const FString& ItemType);
	UFUNCTION(BlueprintCallable)
	void UpdateItemVisuals(class UItemObject* Item);
	void UpdateHUD();

	//Helper functions
	FString GetMeshPathByItemType(const FString& ItemType);
	USkeletalMeshComponent* GetSkeletalMeshComponentByItemType(const FString& ItemType);
	UStaticMeshComponent* GetStaticMeshComponentByItemType(const FString& ItemType);
	void SetReplicatedMeshByItemType(UObject* MeshAsset, const FString& ItemType);
	void OnRep_MeshUpdate(UObject* MeshAsset, const FString& ItemType);

	void TryBindInventoryDelegates();

	//Animations
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimSequence* SwordBasicAttack0;
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimSequence* SwordBasicAttack1;
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimSequence* SwordBasicAttack2;
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimSequence* SwordBasicAttack3;

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

	//Other Classes
	UPROPERTY()
	class UItemMeshData* ItemMeshData;
	UPROPERTY(EditDefaultsOnly, Category = "Item Generation")
	TSubclassOf<class UItemMeshData> ItemMeshDataClass;

	UPROPERTY()
	class UInventory* Inventory;

	void AddExperience(int Amount);

	virtual void OnLevelUp();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void Die();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsPauseMenuVisible();

protected:

	//Other Classes
	UPROPERTY(EditDefaultsOnly, Category = "ItemActor")
	TSubclassOf<class AItemActor> ItemActorClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UInventory> InventoryClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UPauseMenu> PauseMenuClass;
	UPROPERTY()
	class UPauseMenu* PauseMenu;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UPauseMenuWidget> PauseMenuWidgetClass;
	UPROPERTY()
	class UPauseMenuWidget* PauseMenuWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UPlayerHUD> PlayerHUDClass;
	UPROPERTY(BlueprintReadWrite)
	class UPlayerHUD* PlayerHUD;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UAbilityPointCounter> AbilityPointCounterClass;

	UPROPERTY()
	class UAbilityPointCounter* AbilityPointCounter;


	UPROPERTY(EditAnywhere)
	TSubclassOf<class UHealthPotion> HealthPotionClass;

	UPROPERTY()
	class UHealthPotion* HealthPotion;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UAbilitySelection> AbilitySelectionClass;

	UPROPERTY()
	class UAbilitySelection* AbilitySelection;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ULevelUpHUD> LevelUpHUDClass;
	UPROPERTY()
	class ULevelUpHUD* LevelUpHUD;

	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* PlayerHUDWorld;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	class UInventoryComponent* InventoryComponent;

	FTimerHandle DelegateBindHandle;

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateHUD();

	virtual void OnRep_PlayerState() override;

	//Replication
	UFUNCTION()
	void OnRep_HUD();
	UFUNCTION()
	void OnRep_PlayerGold();

	//Stats
	UPROPERTY()
	int PlayerNumInventorySlots;
	UPROPERTY(ReplicatedUsing = OnRep_HUD)
	int PlayerHealth;
	UPROPERTY(ReplicatedUsing = OnRep_HUD)
	int PlayerMaxHealth;
	UPROPERTY(ReplicatedUsing = OnRep_HUD)
	int PlayerResource;
	UPROPERTY(ReplicatedUsing = OnRep_HUD)
	int PlayerMaxResource;
	UPROPERTY(ReplicatedUsing = OnRep_HUD)
	int PlayerXP;
	UPROPERTY(ReplicatedUsing = OnRep_HUD)
	int PlayerXPToLevel;
	UPROPERTY(ReplicatedUsing = OnRep_HUD)
	int PlayerLevel;
	UPROPERTY(ReplicatedUsing = OnRep_HUD)
	int PlayerAbilityPoints;
	UPROPERTY(ReplicatedUsing = OnRep_PlayerGold, BlueprintReadOnly, Category = "Player")
	int PlayerGold;
	UPROPERTY()
	int PlayerArmor;
	UPROPERTY()
	int PlayerDexterity;
	UPROPERTY()
	int PlayerStrength;
	UPROPERTY()
	int PlayerIntelligence;
	UPROPERTY()
	int PlayerStartingMaxHealth;
	UPROPERTY()
	int PlayerStartingMaxResource;
	UPROPERTY()
	int PlayerStartingArmor;
	UPROPERTY()
	int PlayerStartingDexterity;
	UPROPERTY()
	int PlayerStartingStrength;
	UPROPERTY()
	int PlayerStartingIntelligence;
	UPROPERTY()
	int PlayerStartingGold;
	UPROPERTY()
	int HealthPotionCount;   
	UPROPERTY()
	float HealthGranted;         

	float DamageCooldown = 5.f;
	float DefenceCooldown = 5.f;
	float UtilityCooldown = 5.f;
	float UltimateCooldown = 10.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector respawnPoint;

	UPROPERTY()
	UAttributeSet* Attributes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystem;

	virtual void GiveDamageAbilities();
	virtual void GiveDefenceAbilities();
	virtual void GiveUtilityAbilities();
	virtual void GiveUltimateAbilities();

	//Setup
	virtual void BeginPlay();
	void InitLocalUI();

	void SpawnStarterItems();

	//Ticking
	void Tick(float DeltaTime);
	UFUNCTION()
	void PassiveHealthRegen();
	FTimerHandle HealthRegenTimerHandle;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const { return AbilitySystem; }

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

