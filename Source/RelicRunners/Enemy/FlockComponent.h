// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "FlockComponent.generated.h"

class UEnemyBoidMovementComponent;

USTRUCT()
struct FMoveData : public FFastArraySerializerItem
{
	GENERATED_BODY()
	//I am using ID instead of pointer to avoid issues with pointers being invalid when accessed on the client and int32 is more efficient to replicate than a pointer
	//update ID is not shared across clients. FUCK
	UPROPERTY()
	int32 BoidID = -1;
	UPROPERTY()
	FVector Location = FVector::ZeroVector;
	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	void PostReplicatedAdd(const struct FMoveDataArray& InArraySerializer);

	void PostReplicatedChange(const struct FMoveDataArray& InArraySerializer);

	//void PreReplicatedRemove(const struct FMoveDataArray& InArraySerializer);
};

USTRUCT()
struct FMoveDataArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FMoveData> MoveDatas;
	UPROPERTY()
	TObjectPtr<class UFlockComponent> Owner;
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FMoveData, FMoveDataArray>(MoveDatas, DeltaParms, *this);
	}
};

template<>
struct TStructOpsTypeTraits<FMoveDataArray> : public TStructOpsTypeTraitsBase2<FMoveDataArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RELICRUNNERS_API UFlockComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UPROPERTY(Replicated)
	FMoveDataArray MoveDataArray;
	UPROPERTY(Replicated)
	TArray<UEnemyBoidMovementComponent*> BoidMovementComponents;
public:	
	// Sets default values for this component's properties
	UFlockComponent();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void AddBoid(UEnemyBoidMovementComponent* boid);
	void RemoveBoid(UEnemyBoidMovementComponent* boid);
	void UpdateBoidMoveData(float delta);


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
