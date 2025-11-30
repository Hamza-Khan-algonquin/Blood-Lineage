/***************************************************************************************
Class Name:  ATormentedSoulCharacter
Description: Child class of AEnemyCharacter with some additional functionality
Date Last Changed:        2025/11/21
Author:      Thomas Johnson
Contributors:
	Description Of Contributions:
****************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "TormentedSoulCharacter.generated.h"

UCLASS()
class RELICRUNNERS_API ATormentedSoulCharacter : public AEnemyCharacter
{
	friend class UTormentedSoulAnimNotifyState;
	GENERATED_BODY()
public:
	UPROPERTY(Category = "Enemy|Stats", EditAnywhere, BlueprintReadWrite)
	//The attack damage of the enemy
	float AttackDamage;

	UPROPERTY(Category = "Enemy", EditAnywhere, BlueprintReadWrite)
	//The collider object types to be check for with the attack collider
	TArray<TEnumAsByte<EObjectTypeQuery>> AttackQuery;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool CallEditorDebugMessages = true;
#endif

protected:
	UPROPERTY(Category = "Enemy", EditAnywhere, BlueprintReadWrite)
	//If the actor is attacking
	bool IsAttacking;
	UPROPERTY(Category = "Enemy|Component", EditAnywhere, BlueprintReadWrite)
	//The collider that handles attacks
	UCapsuleComponent* AttackOverlapCollider;

public:
	//constructor
	ATormentedSoulCharacter();

	//method override from AActor handles take damage
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Method that is set for the OnOverlap delegate of a collider, that handles dealing damage to the player.", ShortToolTip = "OnOverlap delegate."))
	//Method that is set for the OnOverlap delegate of a collider, that handles dealing damage to the player
	void BasicAttack(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
