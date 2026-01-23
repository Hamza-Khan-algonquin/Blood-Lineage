// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyData.h"
#include "EnemyStats.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RELICRUNNERS_API UEnemyStats : public UActorComponent
{
	GENERATED_BODY()
protected:
	UPROPERTY(Category = "Stats", EditAnywhere, BlueprintReadWrite, Replicated, BlueprintGetter = GetName, BlueprintSetter = SetName)
	FName EnemyName;
	UPROPERTY(Category = "Stats", EditAnywhere, BlueprintReadWrite, Replicated, BlueprintGetter = GetLevel, BlueprintSetter = SetLevel, meta = (ClampMin = 1, UIMin = 1, Delta = 1))
	int EnemyLevel;
	UPROPERTY(Category = "Enemy", EditAnywhere, BlueprintReadWrite, Replicated, BlueprintGetter = GetEnemyType, BlueprintSetter = SetEnemyType, meta = (ExposeOnSpawn = true, NoResetToDefault, DisplayPriority = 0))
	EEnemyType EnemyType;
	UPROPERTY(Category = "Stats", EditAnywhere, BlueprintReadWrite, Replicated, BlueprintGetter = GetMaxHealth, BlueprintSetter = SetMaxHealth, meta = (ClampMin = 0, UIMin = 0, UIMax = 10000, Delta = 10))
	float MaxHealth;
	UPROPERTY(Category = "Stats", EditAnywhere, BlueprintReadWrite, Replicated, BlueprintGetter = GetCurrentHealth, BlueprintSetter = SetCurrentHealth, meta = (ClampMin = 0, UIMin = 0, Delta = 10))
	float CurrentHealth;
	UPROPERTY(Category = "Stats", EditAnywhere, BlueprintReadWrite, Replicated, BlueprintGetter = GetRemainingStunTime, BlueprintSetter = SetRemainingStunTime, meta = (Delta = 0.1))
	float RemainingStunTime;
	UPROPERTY(Category = "Stats", EditAnywhere, Replicated, BlueprintReadWrite, BlueprintGetter = GetChanceToDropItem, BlueprintSetter = SetChanceToDropItem, meta = (ClampMin = 0, ClampMax = 1, Delta = 0.05, ToolTip = "The Chance the enemy has to drop an item on death from 0.0 to 1.0."))
	//The Chance the enemy has to drop an item on death from 0.0 to 1.0
	float ChanceToDropItem;

public:	
	// Sets default values for this component's properties
	UEnemyStats();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	UFUNCTION(BlueprintGetter)
	FName GetName() const { return EnemyName; }
	UFUNCTION(BlueprintGetter)
	int GetLevel() const { return EnemyLevel; }
	UFUNCTION(BlueprintGetter)
	EEnemyType GetEnemyType() const { return EnemyType; }
	UFUNCTION(BlueprintGetter)
	float GetMaxHealth() const { return MaxHealth; }
	UFUNCTION(BlueprintGetter)
	float GetCurrentHealth() const { return CurrentHealth; }
	UFUNCTION(BlueprintGetter)
	float GetRemainingStunTime() const { return RemainingStunTime; }
	UFUNCTION(BlueprintGetter)
	float GetChanceToDropItem() const { return ChanceToDropItem; }

	UFUNCTION(BlueprintSetter)
	void SetName(const FName& NewName) { EnemyName = NewName; }
	UFUNCTION(BlueprintSetter)
	void SetLevel(int NewLevel) { EnemyLevel = NewLevel; }
	UFUNCTION(BlueprintSetter)
	void SetEnemyType(EEnemyType NewType) { EnemyType = NewType; }
	UFUNCTION(BlueprintSetter)
	void SetMaxHealth(float NewMaxHealth) { MaxHealth = NewMaxHealth; }
	UFUNCTION(BlueprintSetter)
	void SetCurrentHealth(float NewCurrentHealth) { CurrentHealth = FMath::Clamp(NewCurrentHealth, 0.0f, MaxHealth); }
	UFUNCTION(BlueprintSetter)
	void SetRemainingStunTime(float NewStunTime) { RemainingStunTime = NewStunTime; }
	UFUNCTION(BlueprintSetter)
	void SetChanceToDropItem(float NewChance) { ChanceToDropItem = FMath::Clamp(NewChance, 0.0f, 1.0f); };

	UFUNCTION(BlueprintCallable)
	void AddToCurrentHealth(float HealthToAdd) { CurrentHealth = FMath::Clamp(CurrentHealth + HealthToAdd, 0.0f, MaxHealth); };
	UFUNCTION(BlueprintCallable)
	void AddToRemainingStunTime(float TimeToAdd) { RemainingStunTime += TimeToAdd; };

	//This is an conditional compilation macro that specifies that the code with in should only be included when compiling for the Unreal Engine Editor. Basically mean it get tosed when building that blasted exe.
#if WITH_EDITOR
	//"Called when a property is changed in the editor" This is a quote from who ever wrote the comment for this function in UObject.h. This method is inherited from that by the way :)
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
