// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IslandSpawningValuesComponent.generated.h"

USTRUCT(BlueprintType)
struct FIslandValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "/Script/RelicRunners.EIslandObjectiveType"))
	int Objectives;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1", ClampMax = "1000", UIMax = "1000"))
	float TileScale = 1;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", UIMin = "0", ClampMax = "100", UIMax = "100", ToolTip = "Percentage in which the tile is spawned walkable. 100% = All floor, 0% = No floor."))
	float FullPercentage = 50.0f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", UIMin = "0", ClampMax = "100", UIMax = "100", ToolTip = "Percentage in which the tile spawned is a hole. 100% = All holes, 0% = No holes. Note: All hole tiles will spawn a modular obstacle."))
	float HolePercentage = 0;

	UPROPERTY(EditAnywhere)
	FString TargetLevel;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ALevelChangeTrigger> LevelChangeTrigger;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class APackedLevelActor> BasicWall;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class APackedLevelActor> CornerWall;

};

USTRUCT()
struct FIslandStartAndEndTile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Packed level actor used as the starting point of the level."));
	TSubclassOf<class APackedLevelActor> LevelStartPackedActor;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Packed level actor used as the ending point of the level."));
	TSubclassOf<class APackedLevelActor> LevelEndPackedActor;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Packed level actor used as the ending point of the level."));
	TSubclassOf<class AIslandBridge> BridgePackedActor;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class APackedLevelActor> StartEndWall;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", UIMin = "0", ClampMax = "1000", UIMax = "1000", ToolTip = "Minimum distance the starting and ending tiles can be from each other."))
	uint8 StartingToEndingMinDistance;
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
	TArray<TSubclassOf<class APackedLevelActor>> ObstacleActor;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APackedLevelActor>> ObstacleWall;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", UIMin = "0", ClampMax = "100", UIMax = "100", ToolTip = "Percentage in which the tile is spawned walkable. 100% = All floor, 0% = No floor."))
	float ObstaclePercentage;
};

USTRUCT()
struct FHoleObstacleValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APackedLevelActor>> HoleTileActor;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APackedLevelActor>> HoleWallMesh;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", UIMin = "0", ClampMax = "100", UIMax = "100", ToolTip = "Percentage in which the tile is spawned walkable. 100% = All floor, 0% = No floor."))
	float HolePercentage = 0.0f;
};

USTRUCT()
struct FVantagePointValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APackedLevelActor>> VantageActor;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APackedLevelActor>> VantageWall;

	UPROPERTY(EditAnywhere)
	uint8 MaxVantageAmountMin = 0;

	UPROPERTY(EditAnywhere)
	uint8 MaxVantageAmountMax = 0;
};

USTRUCT()
struct FObjectiveFlagValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ANewCapturableFlag> Flag;

	UPROPERTY(EditAnywhere)
	uint8 FlagAmountMin = 0;

	UPROPERTY(EditAnywhere)
	uint8 FlagAmountMax = 0;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 FlagWidthMin = 2;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 FlagWidthMax = 2;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 FlagDepthMin = 2;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 FlagDepthMax = 2;

};

USTRUCT()
struct FObjectiveEnemyZoneValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ANewEnemyZone> EnemyZone;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 EnemyZoneWidthMin = 2;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 EnemyZoneWidthMax = 2;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 EnemyZoneDepthMin = 2;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 EnemyZoneDepthMax = 2;

};

USTRUCT()
struct FObjectivePayloadValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf <class ANewPayloadObjective> Payload;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1", ToolTip = "Speed at which the payload moves towards the target."))
	float PayloadSpeed = 1.0f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", UIMin = "0", ClampMax = "1000", UIMax = "1000"))
	float Acceleration = 1.0f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", UIMin = "0", ClampMax = "1000", UIMax = "1000"))
	float Deceleration = 1.0f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 PayloadWidthMin = 2;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 PayloadWidthMax = 2;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 PayloadDepthMin = 2;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 PayloadDepthMax = 2;

};

USTRUCT()
struct FSurvivalZoneValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf <class ANewSurvivalZone> SurvivalZone;

	UPROPERTY(EditAnywhere)
	float SurvivalDuration;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 SurvivalWidthMin = 2;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 SurvivalWidthMax = 2;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 SurvivalDepthMin = 2;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "2", UIMin = "2"))
	uint8 SurvivalDepthMax = 2;

	UPROPERTY(EditAnywhere)
	float SurvivalDuration = 0;

};

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EIslandObjectiveType : uint8
{
	None = 0 UMETA(Hidden),
	CaptureTheFlag = 1 << 0,
	DefeatAllEnemies = 1 << 1,
	MoveThePayload = 1 << 2,
	Survival = 1 << 3,
};
ENUM_CLASS_FLAGS(EIslandObjectiveType);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RELICRUNNERS_API UIslandSpawningValuesComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UIslandSpawningValuesComponent();

public:

	UPROPERTY(Replicated, EditAnywhere)
	FIslandValues IslandValues;

	void SetIslandValues(FIslandValues newValues) { IslandValues = newValues; }

	UPROPERTY(Replicated, EditAnywhere)
	FIslandStartAndEndTile IslandStartEnd;

	void SetIslandStartAndEndValues(FIslandStartAndEndTile newIslandStartEnd) { IslandStartEnd = newIslandStartEnd; }

	UPROPERTY(Replicated, EditAnywhere)
	FIslandFloorMeshes FloorMeshes;

	void SetFloorMeshes(FIslandFloorMeshes newFloorMeshes) { FloorMeshes = newFloorMeshes; }

	UPROPERTY(Replicated, EditAnywhere)
	FNormalObstacleValues ObstacleValues;

	void SetObstacleValues(FNormalObstacleValues newObstacles) { ObstacleValues = newObstacles; }

	UPROPERTY(Replicated, EditAnywhere)
	FHoleObstacleValues HoleValues;

	void SetHoleValues(FHoleObstacleValues newHoleValues) { HoleValues = newHoleValues; }

	UPROPERTY(Replicated, EditAnywhere)
	FVantagePointValues VantageValues;

	void SetVantagePointValues(FVantagePointValues newVantageValues) { VantageValues = newVantageValues; }

	UPROPERTY(Replicated, EditAnywhere)
	FObjectiveFlagValues FlagObjectiveValues;

	void SetFlagObjectiveValues(FObjectiveFlagValues newFlagObjectiveValues) { FlagObjectiveValues = newFlagObjectiveValues; }

	UPROPERTY(Replicated, EditAnywhere)
	FObjectiveEnemyZoneValues EnemyZoneObjectiveValues;

	void SetEnemyZoneObjectiveValues(FObjectiveEnemyZoneValues newEnemyZoneObjectiveValues) { EnemyZoneObjectiveValues = newEnemyZoneObjectiveValues; }

	UPROPERTY(Replicated, EditAnywhere)
	FObjectivePayloadValues PayloadObjectiveValues;

	void SetPayloadObjectiveValues(FObjectivePayloadValues newPayloadObjectiveValues) { PayloadObjectiveValues = newPayloadObjectiveValues; }

	UPROPERTY(Replicated, EditAnywhere)
	FSurvivalZoneValues SurvivalZoneObjectiveValues;

	void SetSurvivalZoneObjectiveValues(FSurvivalZoneValues newSurvivalZoneObjectiveValues) { SurvivalZoneObjectiveValues = newSurvivalZoneObjectiveValues; }

};
