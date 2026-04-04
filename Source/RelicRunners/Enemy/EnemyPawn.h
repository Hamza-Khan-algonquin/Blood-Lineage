// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "RelicRunners/ObjectPool/PoolActorInterface.h"
#include "RelicRunners/Enemy/EnemyData.h"
#include "Components/TimelineComponent.h"
#include "RootPawn.h"
#include "EnemyPawn.generated.h"

class UCapsuleComponent;
class UEnemyLODSkeletalMeshComponent;
class UEnemyLODStaticMeshComponent;
class UEnemyLODComponent;
class UEnemyBoidMovementComponent;
class UEnemyStats;
class USphereComponent;
class UNiagaraSystem;
class USoundBase;

UCLASS()
class RELICRUNNERS_API AEnemyPawn : public APawn, public IPoolActorInterface
{
	GENERATED_BODY()
public:
	FVector CalculateValidDropLocation(FVector DesiredLoc) const;

	UPROPERTY()
	class UItemMeshData* ItemMeshData;

	UPROPERTY(EditDefaultsOnly, Category = "Item Generation")
	TSubclassOf<class UItemMeshData> ItemMeshDataClass;

	//Delegate that is called when the enemy dies
	UPROPERTY(BlueprintAssignable, Category = "Enemy|Delegates")
	FEnemyCharacterDied OnDied;

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Delegates")
	FEnemyCharacterDeactivated OnDeactivated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UEnemyLODSkeletalMeshComponent> SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UEnemyLODStaticMeshComponent> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UEnemyLODComponent> LODComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UEnemyBoidMovementComponent> MovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UEnemyStats> EnemyStatsComponent;

	UPROPERTY(EditAnywhere, Category = "Hit Effect")
	UCurveFloat* HitFlashCurve;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* BasicAttackCollider;*/

	FTimeline HitFlashTimeline;

	FTimerHandle ActiveChangeTimerHandle;

	TArray<UMaterialInstanceDynamic*> DynamicMaterials;

	UPROPERTY(EditAnywhere, Category = "Hit Effect")
	float KnockBackStrength;
	UPROPERTY(EditAnywhere, Category = "Hit Effect")
	float AdditionalUpwardsKnockBackForces;
	bool IsAttacking = false;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* DeathEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* HitSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* DeathSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* AttackHitPlayerSound = nullptr;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "ItemActor")
	TSubclassOf<class AItemActor> ItemActorClass;

	//interface implementation variable
	UPROPERTY(Category = "Object Pooling Info", EditAnywhere, BlueprintReadWrite)//, ReplicatedUsing = "OnRep_IsActive")
	bool IsActive = true;
	UPROPERTY(Category = "Object Pooling Info", EditAnywhere, BlueprintReadWrite)
	bool SetControllerActiveStatus = true;
	UPROPERTY(Category = "Object Pooling Info", EditAnywhere, BlueprintReadWrite)
	FTimerHandle LifeSpanTimerHandle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> AttackMontage;
	FTimerHandle AttackHandle;


public:
	// Sets default values for this pawn's properties
	AEnemyPawn();

	void UpdateEmissiveHealth();
	void EndPlay(const EEndPlayReason::Type EndPlayReason);
	//method to make sure we are setting up replication properly

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Override of AActor's TakeDamage function to handle damage to the enemy."))
	//Override of AActor's TakeDamage function to handle damage to the enemy
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	UFUNCTION(BlueprintCallable, Reliable, NetMulticast)
	void HitFlash();
	UFUNCTION(BlueprintCallable)
	void UpdateHitFlash(float strength);
	UFUNCTION(BlueprintCallable)
	void EndHitFlash();

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "A method to handle dieing and what happens when the enemy dies.", ShortToolTip = "A method to handle dieing."))
	//A method to handle dieing and what happens when the enemy dies
	virtual void Die(AController* EventInstigator, FVector hitDirection = FVector(0,0,0));
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDeathEffect(FVector impactDirection);
	UFUNCTION(BlueprintCallable, Reliable, NetMulticast)
	void Multicast_GoldStuff();

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Check if the enemy is dead.", ShortToolTip = "Check if the enemy is dead."))
	//Check if the enemy is dead
	virtual bool IsDead();

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Reset enemy variables.", ShortToolTip = "Hi."))
	void ResetEnemy();

	void AddEnemyToDirector();

	void StartAttack();
	void AttackTick();
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Method that is set for the OnOverlap delegate of a collider, that handles dealing damage to the player.", ShortToolTip = "OnOverlap delegate."))
	void BasicAttack(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual float CalculateXP() const;

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "A method to handle spawning an item on death using infromation from the provided loot pool.", ShortToolTip = "A method to handle spawning an item on death."))
	//A method to handle spawning an item on death using infromation from the provided loot pool
	virtual void SpawnItem(int KillersLuck);

	UFUNCTION(BlueprintCallable)
	void ApplyForceToEnemy(FVector forces);

	UFUNCTION(BlueprintCallable, Reliable, NetMulticast)
	void KnockBack(FVector forces);

	UFUNCTION(BlueprintCallable)
	void GroundCheck(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	//best method name ever
	UFUNCTION(BlueprintCallable, Reliable, NetMulticast)
	void ResetKnockBackStuff();

	UFUNCTION(NetMulticast, Reliable)
	void NetMultSetIsActive(bool newIsActive);

	UFUNCTION()
	void OnRep_IsActive();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
public:
	//IPoolActorInterface implementation
	virtual bool GetIsActive() override;
	virtual bool GetShouldTick() override;
	virtual void SetIsActive(bool newIsActive) override;
	virtual FTimerHandle& GetLifeSpanTimerHandle() override;
};
