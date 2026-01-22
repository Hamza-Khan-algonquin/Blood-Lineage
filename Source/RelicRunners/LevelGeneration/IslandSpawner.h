// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IslandSpawner.generated.h"

UCLASS()
class RELICRUNNERS_API AIslandSpawner : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AIslandSpawner();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UIslandSpawningValuesComponent* GenerationValuesComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "GenerationValues")
	TSubclassOf<class AIslandGenerationTest> IslandGenerationTestTemplate;

	TArray<class AIslandGenerationTest*> Islands;

	int ObjectiveType;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values", meta = (ClampMin = "1", UIMin = "1"))
	uint8 NumIslands;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values", meta = (ClampMin = "1", UIMin = "1", ClampMax = "1000", UIMax = "1000"))
	float TileScale;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values| Map Size", meta = (ClampMin = "2", UIMin = "2"))
	uint8 WidthMin;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values| Map Size", meta = (ClampMin = "2", UIMin = "2"))
	uint8 WidthMax;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values| Map Size", meta = (ClampMin = "2", UIMin = "2"))
	uint8 DepthMin;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values| Map Size", meta = (ClampMin = "2", UIMin = "2"))
	uint8 DepthMax;


	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Floor", meta = (ToolTip = "Mesh used for the quarter of a walkable tile with neighbours."))
	class UStaticMesh* FullFloorMesh;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Floor", meta = (ToolTip = "Mesh used for the quarter of a walkable tile with neighbours."))
	class UStaticMesh* SideFloorMesh;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Floor", meta = (ToolTip = "Mesh used for the quarter of a walkable tile with neighbours."))
	class UStaticMesh* ConvexCornerMesh;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Floor", meta = (ToolTip = "Mesh used for the quarter of a walkable tile with neighbours."))
	class UStaticMesh* SideFloorWallMesh;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Floor", meta = (ToolTip = "Mesh used for the quarter of a walkable tile with neighbours."))
	class UStaticMesh* ConvexCornerWallMesh;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Floor", meta = (ToolTip = "Mesh used for the quarter of a walkable tile with neighbours."))
	class UMaterial* FloorMaterial;


	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Obstacle", meta = (ToolTip = "Packed level actor used as the starting point of the level."));
	TSubclassOf<class APackedLevelActor> LevelStartPackedActor;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Obstacle", meta = (ToolTip = "Packed level actor used as the ending point of the level."));
	TSubclassOf<class APackedLevelActor> LevelEndPackedActor;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Obstacle", meta = (ToolTip = "Array of regular packed level actors that the level generator will pick out of randomly to spawn."));
	TSet <TSubclassOf<class APackedLevelActor>> BasicPackedObstacles;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Obstacle", meta = (ToolTip = "Array of regular packed level actors that the level generator will pick out of randomly to spawn."));
	TSet <TSubclassOf<class APackedLevelActor>> BasicPackedObstacleWalls;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Obstacle", meta = (ToolTip = "Array of regular packed level actors that the level generator will pick out of randomly to spawn."));
	TSet <TSubclassOf<class APackedLevelActor>> BasicHolePackedObstacles;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Obstacle", meta = (ToolTip = "Array of regular packed level actors that the level generator will pick out of randomly to spawn."));
	TSet <TSubclassOf<class APackedLevelActor>> BasicHolePackedObstacleWalls;


	UPROPERTY(EditAnywhere, Category = "GenerationValues | Objective Values| Capture the Flag", meta = (ToolTip = "Array of Packed level actor used as the *Capture the flag* objective points."));
	TSubclassOf<class APackedLevelActor> CapturableFlagPackedActorArray;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Objective Values| Capture the Flag", meta = (ToolTip = "Maximum amount of tiles in which a flag objective is spawned on top of. Tile that are set as unwalkable are overriden to be walkable."))
	uint8 MaxCapturableFlagAmount;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Objective Values| Enemies", meta = (ToolTip = "Array of Packed level actor used as the *Enemy Zone* objective points."));
	TSubclassOf<class APackedLevelActor> EnemyZonePackedActorArray;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Objective Values| Enemies", meta = (ToolTip = "Maximum amount of tiles in which an enemy zone objective is spawned on top of. Tile that are set as unwalkable are overriden to be walkable."))
	uint8 MaxEnemyZoneAmount;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Objective Values| Move the Payload")
	TSubclassOf<class APayloadActor> PayloadActor;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Objective Values| Move the Payload", meta = (ClampMin = "1", UIMin = "1", ToolTip = "Speed at which the payload moves towards the target."))
	float PayloadMovementSpeed;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Objective Values| Survival Zone", meta = (ToolTip = "Array of Packed level actor used as the *Survival Zone* objective points."));
	TSubclassOf<class APackedLevelActor> SurvivalZonePackedActorArray;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Objective Values| Survival Zone", meta = (ToolTip = "Maximum amount of tiles in which a survival zone objective is spawned on top of. Tile that are set as unwalkable are overriden to be walkable."))
	uint8 MaxSurvivalZoneAmount;

public:

	int GetObjectiveValue();

};