/***************************************************************************************
Class Name:  AEnemyCharacter
Description: This Class is the base class for all enemies with more complicated ai as it inherits from ACharacter and not APawn.
Date Last Changed:        2025/11/21
Author:      Thomas Johnson
Contributors:
	Description Of Contributions:
****************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RelicRunners/ObjectPool/PoolActorInterface.h"
#include "RelicRunners/Enemy/EnemyData.h"
#include "EnemyCharacter.generated.h"

/*
To Do:
	- Add a method that run a animation both on client and server that work like play montage in blueprints
*/

class UWidgetComponent;
class UAnimMontage;
class UEnemyStats;
class UNiagaraSystem;
class UHitFlashComponent;
class UHitStunComponent;

UCLASS(Blueprintable, Abstract)
class RELICRUNNERS_API AEnemyCharacter : public ACharacter, public IPoolActorInterface
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool ShouldDieIfCommanderIsDone;
	//Delegate that is called when the enemy dies
	UPROPERTY(BlueprintAssignable, Category = "Enemy|Delegates")
	FEnemyCharacterDied OnDied;

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Delegates")
	FEnemyCharacterDeactivated OnDeactivated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UEnemyStats> EnemyStatsComponent;
	UPROPERTY(EditAnywhere)
	bool ShouldTakeDamage;

	UPROPERTY(EditAnywhere)
	float HitStunTime;
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UNiagaraSystem> DamageEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UNiagaraSystem> DeathEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<struct FItemData> ItemLootPool;

	UPROPERTY()
	class UItemMeshData* ItemMeshData;
	UPROPERTY(EditDefaultsOnly, Category = "Item Generation")
	TSubclassOf<class UItemMeshData> ItemMeshDataClass;

	UPROPERTY(Category = "Enemy|Component", VisibleAnywhere, Replicated, NoClear, meta = (DisplayThumbnail = true))
	UWidgetComponent* EnemyHUDWorld;
	UPROPERTY(EditDefaultsOnly, Category = "ItemActor")
	TSubclassOf<class AItemActor> ItemActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UHitFlashComponent> HitFlashComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UHitStunComponent> HitStunComponent;

	UPROPERTY(EditAnywhere)
	bool OverrideCollisionType = false;
	

	//interface implementation variable
	UPROPERTY(Category = "Object Pooling Info", Replicated, EditAnywhere, BlueprintReadWrite)
	bool IsActive = true;
	UPROPERTY(Category = "Object Pooling Info", EditAnywhere, BlueprintReadWrite)
	bool SetControllerActiveStatus = true;
	UPROPERTY(Category = "Object Pooling Info", EditAnywhere, BlueprintReadWrite)
	FTimerHandle LifeSpanTimerHandle;

public:
	// Sets default values for this character's properties
	AEnemyCharacter();

	//method to make sure we are setting up replication properly
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Override of AActor's TakeDamage function to handle damage to the enemy."))
	//Override of AActor's TakeDamage function to handle damage to the enemy
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "A method to handle dieing and what happens when the enemy dies.", ShortToolTip = "A method to handle dieing."))
	//A method to handle dieing and what happens when the enemy dies
	virtual void Die(AController* EventInstigator);

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Check if the enemy is dead.", ShortToolTip = "Check if the enemy is dead."))
	//Check if the enemy is dead
	virtual bool IsDead();

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Reduces the remaining stun time by DeltaTime. Meant to be called in Tick but can be called elsewhere if needed.", ShortToolTip = "Check if the enemy is dead."))
	//Reduces the remaining stun time by DeltaTime. Meant to be called in Tick but can be called elsewhere if needed.
	void ReduceStunTime(float DeltaTime);

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Reset enemy variables.", ShortToolTip = "Hi."))
	void ResetEnemy();

	void AddEnemyToDirector();

	//Need to be netmulticast to play on all clients as client does own the actor their for client rpc won't work
	//Runs a montage client side
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, meta = (ToolTip = "Runs a montage client side. Need to be netmulticast to play on all clients as client does own the actor their for client rpc won't work.", ShortToolTip = "Runs a montage client side"))
	void PlayMontageOnClient(UAnimMontage* Montage, float PlayRate = 1.0f);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "A method to handle spawning an item on death using infromation from the provided loot pool.", ShortToolTip = "A method to handle spawning an item on death."))
	//A method to handle spawning an item on death using infromation from the provided loot pool
	virtual void SpawnItem();

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Calculates the amount of XP the enemy should give on death.", ShortToolTip = "Calculates the amount of XP the enemy should giveS."))
	//Calculates the amount of XP the enemy should give on death.
	virtual float CalculateXP() const;

	//rotates hud to face camera, courtesy of Mr big T him self Tristan
	void UpdateEnemyHUDWorldFacing();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void NetMultPlayParticleEffect(UNiagaraSystem* effect, FVector position);

	UFUNCTION(NetMulticast, Reliable)
	void NetMultSetIsActive(bool newIsActive);

	void Destroy();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

//Interface Implementation
public:
	//IPoolActorInterface implementation
	virtual bool GetIsActive() override;
	virtual bool GetShouldTick() override;
	virtual void SetIsActive(bool newIsActive) override;
	virtual FTimerHandle& GetLifeSpanTimerHandle() override;

};
