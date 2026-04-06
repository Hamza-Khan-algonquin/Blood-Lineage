// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyLODComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpdateLOD, int, NewLOD, int, OldLOD);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RELICRUNNERS_API UEnemyLODComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	FOnUpdateLOD OnUpdateLODEvent;
protected:
	//Current LOD level
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LOD")
	int CurrentLOD = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ToolTip = "Will use LOD if distance between ai and player is less than or equal to number"))
	TArray<float> DistanceOfLOD;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LOD")
	bool IsUpdatingLOD = true;

public:	
	// Sets default values for this component's properties
	UEnemyLODComponent();

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Called to check and update the LOD."))
	void UpdateLOD();
	UFUNCTION(BlueprintCallable, Unreliable, NetMulticast)
	void MultiCastUpdateLOD();

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Getter for the current LOD level."))
	int GetCurrentLODLevel() const { return CurrentLOD; }

	UFUNCTION(BlueprintCallable)
	void SetLOD(int newLOD);

	UFUNCTION(BlueprintCallable, Unreliable, NetMulticast)
	void MultiCastSetLOD(int newLOD);

	UFUNCTION(BlueprintCallable)
	void SetIsUpdatingLOD(bool isUpdating);
	float GetDistanceLOD(int index) { return DistanceOfLOD[index]; }
	int GetNumberOfLODs() { return DistanceOfLOD.Num(); }
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	//Used for checking and updating LOD level. Can disable ticking if another system is used to call this function periodically.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
