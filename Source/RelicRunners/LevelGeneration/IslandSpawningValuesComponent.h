// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IslandSpawningValuesComponent.generated.h"

USTRUCT()
struct FIslandValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1", ClampMax = "1000", UIMax = "1000"))
	float TileScale = 1;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", UIMin = "0", ClampMax = "100", UIMax = "100", ToolTip = "Percentage in which the tile is spawned walkable. 100% = All floor, 0% = No floor."))
	float FullPercentage = 50.0f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 WidthMin = 2;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 WidthMax = 2;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 DepthMin = 2;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 DepthMax = 2;

};

USTRUCT()
struct FIslandFloorMeshes
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	UStaticMesh* FullFloorMesh = nullptr;

	UPROPERTY(EditAnywhere)
	UStaticMesh* SideFloorMesh = nullptr;

	UPROPERTY(EditAnywhere)
	UStaticMesh* CornerFloorMesh = nullptr;

	UPROPERTY(EditAnywhere)
	UMaterial* FloorMaterial = nullptr;
};

USTRUCT()
struct FNormalObstacleValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSet<TSubclassOf<class APackedLevelActor>> ObstacleMesh;

	UPROPERTY(EditAnywhere)
	TSet<UStaticMesh*> WallMesh;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", UIMin = "0", ClampMax = "100", UIMax = "100", ToolTip = "Percentage in which the tile is spawned walkable. 100% = All floor, 0% = No floor."))
	float ObstaclePercentage;
};

USTRUCT()
struct FHoleObstacleValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSet<TSubclassOf<class APackedLevelActor>> HoleTileMesh;

	UPROPERTY(EditAnywhere)
	TSet<UStaticMesh*> HoleWallMesh;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", UIMin = "0", ClampMax = "100", UIMax = "100", ToolTip = "Percentage in which the tile is spawned walkable. 100% = All floor, 0% = No floor."))
	float HolePercentage = 0.0f;
};

USTRUCT()
struct FVantagePointValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSet<TSubclassOf<class APackedLevelActor>> VantageMesh;

	UPROPERTY(EditAnywhere)
	TSet<UStaticMesh*> VantageWallMesh;

	UPROPERTY(EditAnywhere)
	uint8 MaxVantageAmountMin = 0;

	UPROPERTY(EditAnywhere)
	uint8 MaxVantageAmountMax = 0;

};

USTRUCT()
struct FObjectiveFlagValues
{
	GENERATED_BODY()

	//UPROPERTY(EditAnywhere)
	//ACapturableFlag* Flag;

	UPROPERTY(EditAnywhere)
	uint8 FlagAmountMin = 0;

	UPROPERTY(EditAnywhere)
	uint8 FlagAmountMax = 0;

};

USTRUCT()
struct FObjectiveEnemyZoneValues
{
	GENERATED_BODY()

	//UPROPERTY(EditAnywhere)
	//AEnemyZone* EnemyZone;

	UPROPERTY(EditAnywhere)
	uint8 FlagAmountMin = 0;

	UPROPERTY(EditAnywhere)
	uint8 FlagAmountMax = 0;

};

USTRUCT()
struct FObjectivePayloadValues
{
	GENERATED_BODY()

	//UPROPERTY(EditAnywhere)
	//APayloadActor* Payload

	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1", ToolTip = "Speed at which the payload moves towards the target."))
	float PayloadSpeed = 1.0f;

};

USTRUCT()
struct FSurvivalZoneValues
{
	GENERATED_BODY()

	//UPROPERTY(EditAnywhere)
	//ASurvivalZone* SurvivalZone

	UPROPERTY(EditAnywhere)
	uint8 ZoneAmountMin = 0;

	UPROPERTY(EditAnywhere)
	uint8 ZoneAmountMax = 0;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RELICRUNNERS_API UIslandSpawningValuesComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UIslandSpawningValuesComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere)
	FIslandValues IslandValues;

	void SetIslandValues(FIslandValues newValues) { IslandValues = newValues; }

	UPROPERTY(EditAnywhere)
	FIslandFloorMeshes FloorMeshes;

	void SetFloorMeshes(FIslandFloorMeshes newFloorMeshes) { FloorMeshes = newFloorMeshes; }

	UPROPERTY(EditAnywhere)
	FNormalObstacleValues ObstacleValues;

	void SetObstacleValues(FNormalObstacleValues newObstacles) { ObstacleValues = newObstacles; }

	UPROPERTY(EditAnywhere)
	FHoleObstacleValues HoleValues;

	void SetHoleValues(FHoleObstacleValues newHoleValues) { HoleValues = newHoleValues; }

	UPROPERTY(EditAnywhere)
	FVantagePointValues VantageValues;

	void SetVantagePointValues(FVantagePointValues newVantageValues) { VantageValues = newVantageValues; }

	UPROPERTY(EditAnywhere)
	FObjectiveFlagValues FlagObjectiveValues;

	void SetFlagObjectiveValues(FObjectiveFlagValues newFlagObjectiveValues) { FlagObjectiveValues = newFlagObjectiveValues; }

	UPROPERTY(EditAnywhere)
	FObjectiveEnemyZoneValues EnemyZoneObjectiveValues;

	void SetEnemyZoneObjectiveValues(FObjectiveEnemyZoneValues newEnemyZoneObjectiveValues) { EnemyZoneObjectiveValues = newEnemyZoneObjectiveValues; }

	UPROPERTY(EditAnywhere)
	FObjectivePayloadValues PayloadObjectiveValues;

	void SetPayloadObjectiveValues(FObjectivePayloadValues newPayloadObjectiveValues) { PayloadObjectiveValues = newPayloadObjectiveValues; }

	UPROPERTY(EditAnywhere)
	FSurvivalZoneValues SurvivalZoneObjectiveValues;

	void SetSurvivalZoneObjectiveValues(FSurvivalZoneValues newSurvivalZoneObjectiveValues) { SurvivalZoneObjectiveValues = newSurvivalZoneObjectiveValues; }

};
