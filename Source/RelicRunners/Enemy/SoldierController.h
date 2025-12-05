/***************************************************************************************
Class Name:  ASoldierController
Description: A controller class that is meant to be used as the base class for any npc that will fight in the game
Date Last Changed:        2025/11/21
Author:      Thomas Johnson
Contributors:
	Description Of Contributions:
****************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RelicRunners/ObjectPool/PoolActorInterface.h"
#include "SoldierController.generated.h"

class UAIPerceptionComponent;
class UBehaviorTree;

UCLASS()
class RELICRUNNERS_API ASoldierController : public AAIController, public IPoolActorInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = "Enemy|Component", EditAnywhere, BlueprintReadWrite)
	//The preception component that handles detecting targets for the soldier, like the player for the enemies
	UAIPerceptionComponent* AIPerceptionComp;
protected:
	UPROPERTY(Category = "Enemy", VisibleAnywhere, BlueprintReadWrite)
	//A bool that keep track of whether the soldier 
	bool IsInCombat;
	UPROPERTY(Category = "Enemy", EditAnywhere, BlueprintReadWrite)
	//The radius the enemy will patrol around the patrol point
	float PatrolRadius;
	UPROPERTY(Category = "Enemy", EditAnywhere, BlueprintReadWrite)
	//The origin the control will use to find the navmesh to use
	FVector OriginPatrolZone;
	UPROPERTY(Category = "Enemy", EditAnywhere, BlueprintReadWrite)
	//The point that the soldier will patrol around
	FVector CurrentPatrolCenter;

	UPROPERTY(Category = "Enemy", EditAnywhere, BlueprintReadOnly)
	//The Behavoir tree to use on being play
	UBehaviorTree* StartingBehaviorTree;

	//object pooling variables
	UPROPERTY(Category = "Object Pooling Info", EditAnywhere, BlueprintReadWrite)
	bool IsActive;
	UPROPERTY(Category = "Object Pooling Info", EditAnywhere, BlueprintReadWrite)
	FTimerHandle LifeSpanTimerHandle;

public:
	//The default constructor
	ASoldierController();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "The getter for IsInCombat.", ShortToolTip = "The getter for IsInCombat."))
	//The getter for IsInCombat
	bool GetIsInCombat() const;

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "A check to make sure the owned pawn is still with in the range of patrol around the patrol point.", ShortToolTip = "Check if in patrol radius."))
	//A check to make sure the owned pawn is still with in the range of patrol around the patrol point
	bool IsInPatrolRange();
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Will return a random point within the patrol radius.", ShortToolTip = "Get a random patrol point."))
	//Will return a random point within the patrol radius
	FVector GetRandomPatrolPoint();
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "A Getter for the current.", ShortToolTip = "A Getter for the current."))
	//A Getter for the current 
	FVector GetCurrentPatrolCenter() const { return CurrentPatrolCenter; };

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Getter for the current patrol center.", ShortToolTip = "Getter for the current patrol center."))
	//Getter for the current patrol center
	void SetCurrentPatrolCenter(const FVector &NewPatrolPoint) { CurrentPatrolCenter = NewPatrolPoint; }
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Getter for the patrol zone origin.", ShortToolTip = "Getter for the patrol zone origin."))
	//Getter for the patrol zone origin
	void SetOriginPatrolZone(const FVector &NewCenter) { OriginPatrolZone = NewCenter; }

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

//Interface Implementation
public:
	//IPoolActorInterface implementation
	virtual bool GetIsActive() override;
	virtual bool GetShouldTick() override;
	virtual void SetIsActive(bool newIsActive) override;
	virtual FTimerHandle& GetLifeSpanTimerHandle() override;
};
