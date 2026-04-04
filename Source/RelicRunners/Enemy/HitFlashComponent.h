// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "HitFlashComponent.generated.h"

class UCurveFloat;
class UMaterialInstanceDynamic;
class USkeletalMeshComponent;
struct FColor;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RELICRUNNERS_API UHitFlashComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	//probably does not need to be a soft pointer as HitFlashTimeline keeps it load but is just incase.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr <UCurveFloat> HitFlashCurve;

	//The timeline that will be used to drive the hit flash effect. It will use the HitFlashCurve to determine the strength of the hit flash at any given time.
	UPROPERTY(VisibleAnywhere)
	FTimeline HitFlashTimeline;
	
	//The dynamic materials that will be used to apply the hit flash effect. Call InitializeNewDynamicMaterials add pass the mesh you want the effect to have to use it
	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;

	UPROPERTY(EditAnywhere, Replicated)
	FColor HitFlashColor;
public:	
	// Sets default values for this component's properties
	UHitFlashComponent();
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	//Called when the component ends play and it is at the end of it lifespan
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
	// Called when the game starts
	virtual void BeginPlay() override;
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
public:
	UFUNCTION(BlueprintCallable)
	void StartHitFlash();
	UFUNCTION(BlueprintCallable, Reliable, NetMulticast)
	void Multicast_StartHitFlash();
	UFUNCTION(BlueprintCallable)
	void EndHitFlash();
	UFUNCTION(BlueprintCallable, Reliable, NetMulticast)
	void Multicast_EndHitFlash();
	
	UFUNCTION(BlueprintCallable)
	void InitializeNewDynamicMaterials(UMeshComponent* newComponent);
	UFUNCTION(BlueprintCallable, Reliable, NetMulticast)
	void Multicast_InitializeNewDynamicMaterials(UMeshComponent* newComponent);
	UFUNCTION(BlueprintCallable)
	void RemoveNewDynamicMaterials(UMeshComponent* removedComponent);
	UFUNCTION(BlueprintCallable, Reliable, NetMulticast)
	void Multicast_RemoveNewDynamicMaterials(UMeshComponent* removedComponent);
	UFUNCTION(BlueprintCallable)
	void SetHitFlashColor(const FColor newColor);
protected:
	UFUNCTION(BlueprintCallable)
	void UpdateHitFlash(const float strength);
	
	UFUNCTION(BlueprintCallable, Reliable, NetMulticast)
	void UpdateDynamicMaterialColors();

	

		
};
