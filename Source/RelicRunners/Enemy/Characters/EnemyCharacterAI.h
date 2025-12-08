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
#include "EnemyCharacterAI.generated.h"

UCLASS()
class RELICRUNNERS_API AEnemyCharacterAI : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacterAI();

	UPROPERTY(VisibleAnywhere, meta = (DisplayThumbnail = true))
	USkeletalMeshComponent* LowerMesh;

	UPROPERTY(VisibleAnywhere, meta = (DisplayThumbnail = true))
	USkeletalMeshComponent* UpperMesh;

	UPROPERTY(VisibleAnywhere, meta = (DisplayThumbnail = true))
	USkeletalMeshComponent* HelmetMesh;

	UPROPERTY(VisibleAnywhere, meta = (DisplayThumbnail = true))
	USkeletalMeshComponent* ArmsMesh;

	UPROPERTY(VisibleAnywhere, meta = (DisplayThumbnail = true))
	USkeletalMeshComponent* NecklaceMesh;

	UPROPERTY(VisibleAnywhere, meta = (DisplayThumbnail = true))
	UStaticMeshComponent* MainhandItemMesh;

	UPROPERTY(VisibleAnywhere, meta = (DisplayThumbnail = true))
	UStaticMeshComponent* OffhandItemMesh;

	UPROPERTY(VisibleAnywhere, meta = (DisplayThumbnail = true))
	class UWidgetComponent* EnemyHUDWorld;

	UPROPERTY(EditDefaultsOnly, Category = "ItemActor")
	TSubclassOf<class AItemActor> ItemActorClass;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void Die(AController* KillerController);

	int CalculateXPReward() const;

	//Replication
	UFUNCTION()
	void OnRep_CurrentHealth();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	//Stats
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Enemy")
	int EnemyLevel = 1;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Enemy")
	FName EnemyName = "Grunt";

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	int EnemyHealth = 100;

	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth)
	int EnemyMaxHealth = 100;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Enemy")
	bool Alive = true;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Enemy")
	int EnemyResource = 100;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Enemy")
	int EnemyMaxResource = 100;

	UPROPERTY()
	int EnemyArmor = 0;
	UPROPERTY()
	int EnemyDexterity = 0;
	UPROPERTY()
	int EnemyStrength = 0;
	UPROPERTY()
	int EnemyIntelligence = 0;

	int GetEnemyHealth() { return EnemyHealth; }
	int GetEnemyMaxHealth() { return EnemyMaxHealth; }
	int GetEnemyResource() { return EnemyResource; }
	int GetEnemyMaxResource() { return EnemyMaxResource; }
	int GetEnemyLevel() { return EnemyLevel; }
	FName GetEnemyName() { return EnemyName; }
	void TryBindInventoryDelegates();
	void UpdateItemVisuals(UObject* MeshAsset, const FString& ItemType);
	void UpdateItemVisuals(class UItemObject* Item);

	FString GetMeshPathByItemType(const FString& ItemType);
	USkeletalMeshComponent* GetSkeletalMeshComponentByItemType(const FString& ItemType);
	UStaticMeshComponent* GetStaticMeshComponentByItemType(const FString& ItemType);

	UFUNCTION(Client, Reliable)
	void Client_UpdateEquippedStats(const FEquippedStatsSummary& NewStats);

	UFUNCTION(Server, Reliable)
	void Server_SetReplicatedMeshByItemType(UObject* MeshAsset, const FString& ItemType);
	UFUNCTION(Server, Reliable)
	void Server_EquipItemByID(FGuid ID);
	UFUNCTION(Server, Reliable)
	void Server_UnequipItemByID(FGuid ID);

	void SetMaxHealth(int health);

	void OnRep_MeshUpdate(UObject* MeshAsset, const FString& ItemType);

	void SetReplicatedMeshByItemType(UObject* MeshAsset, const FString& ItemType);
	UFUNCTION()
	void OnRep_MaxHealth();
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

	UPROPERTY()
	class UItemMeshData* ItemMeshData;
	UPROPERTY(EditDefaultsOnly, Category = "Item Generation")
	TSubclassOf<class UItemMeshData> ItemMeshDataClass;
	UPROPERTY()
	class UInventory* Inventory;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	class UInventoryComponent* InventoryComponent;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void InitializeStartingEquipment();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void UpdateEnemyHUDWorldFacing();

};
