// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "RelicRunners/ObjectPool/PoolActorInterface.h"
#include "RelicRunners/Enemy/EnemyData.h"
#include "EnemyPawn.generated.h"

class UCapsuleComponent;
class UEnemyLODSkeletalMeshComponent;
class UEnemyLODStaticMeshComponent;
class UEnemyLODComponent;
class UEnemyBoidMovementComponent;

UCLASS()
class RELICRUNNERS_API AEnemyPawn : public APawn, public IPoolActorInterface
{
	GENERATED_BODY()
public:
	//Delegate that is called when the enemy dies
	UPROPERTY(BlueprintAssignable, Category = "Enemy|Delegates")
	FEnemyCharacterDied OnDied;

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Delegates")
	FEnemyCharacterDeactivated OnDeactivated;

	UPROPERTY(Category = "Enemy|Stats", EditAnywhere, BlueprintReadWrite, Replicated, BlueprintSetter = SetCurrentHealth, meta = (ClampMin = 0, UIMin = 0, Delta = 10))
	float CurrentHealth;
	UPROPERTY(Category = "Enemy|Stats", EditAnywhere, BlueprintReadWrite, Replicated, BlueprintSetter = SetMaxHealth, meta = (ClampMin = 0, UIMin = 0, UIMax = 10000, Delta = 10))
	float MaxHealth;
	UPROPERTY(Category = "Enemy|Stats", EditAnywhere, BlueprintReadWrite, Replicated, meta = (Delta = 0.1))
	float AttackDamage = 10;
	UPROPERTY(Category = "Enemy", EditAnywhere, BlueprintReadWrite, Replicated, meta = (ExposeOnSpawn = true, NoResetToDefault, DisplayPriority = 0))
	EEnemyType TypeOfEnemy = EEnemyType::LIGHT;

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

protected:
	//interface implementation variable
	UPROPERTY(Category = "Object Pooling Info", EditAnywhere, BlueprintReadWrite)
	bool IsActive = true;
	UPROPERTY(Category = "Object Pooling Info", EditAnywhere, BlueprintReadWrite)
	bool SetControllerActiveStatus = true;
	UPROPERTY(Category = "Object Pooling Info", EditAnywhere, BlueprintReadWrite)
	FTimerHandle LifeSpanTimerHandle;
	bool IsAttacking = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> AttackMontage;
public:
	// Sets default values for this pawn's properties
	AEnemyPawn();

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
	virtual bool IsDead() { return CurrentHealth <= 0; }

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Getter for the current health.", ShortToolTip = "Getter for the current health."))
	//Getter for the current health
	float GetCurrentHealth() const { return CurrentHealth; }
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Getter for the max health.", ShortToolTip = "Getter for the max health."))
	//Getter for the max health
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Getter for the current resource.", ShortToolTip = "Getter for the current resource."))
	EEnemyType GetEnemyType() const { return TypeOfEnemy; }

	UFUNCTION(BlueprintCallable, BlueprintSetter, meta = (ToolTip = "Getter for the current health.", ShortToolTip = "Getter for the current health."))
	//Setter for the current health
	void SetCurrentHealth(const float& NewHealth);
	UFUNCTION(BlueprintCallable, BlueprintSetter, meta = (ToolTip = "Setter for the max health.", ShortToolTip = "Setter for the max health."))
	//Setter for the max health
	void SetMaxHealth(const float& newHealth);

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Reset enemy variables.", ShortToolTip = "Hi."))
	void ResetEnemy();

	void AddEnemyToDirector();

	void StartAttack();
	void AttackTick();
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Method that is set for the OnOverlap delegate of a collider, that handles dealing damage to the player.", ShortToolTip = "OnOverlap delegate."))
	void BasicAttack(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual float CalculateXP() const;
	
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
