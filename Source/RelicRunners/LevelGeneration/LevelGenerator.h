// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../RelicRunnersGameState.h"
#include "LevelGenerator.generated.h"

enum class EObjectiveType : uint8;

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EFloorNeighbours : uint8
{
	Blank				= 0,
	TopLeft				= 1 << 0,		/*0000 0001*/
	TopMiddle			= 1 << 1,		/*0000 0010*/
	TopRight			= 1 << 2,		/*0000 0100*/
	MiddleLeft			= 1 << 3,		/*0000 1000*/
	MiddleRight			= 1 << 4,		/*0001 0000*/
	BottomLeft			= 1 << 5,		/*0010 0000*/
	BottomMiddle		= 1 << 6,		/*0100 0000*/
	BottomRight			= 1 << 7,		/*1000 0000*/
};
ENUM_CLASS_FLAGS(EFloorNeighbours);

UENUM(BlueprintType)
enum class EFloorObstacle : uint8
{
	None,
	Basic,
	Hole,
	CapturableFlag,
	EnemyZone,
	Start,
	End,
};


USTRUCT()
struct FSFloorValues
{
	GENERATED_BODY()
public:
	UPROPERTY()
	bool IsFullTile = false;

	UPROPERTY()
	EFloorNeighbours FloorNeighbours = EFloorNeighbours::Blank;

	UPROPERTY()
	EFloorNeighbours ObstacleNeighbours = EFloorNeighbours::Blank;

	UPROPERTY()
	EFloorObstacle FloorObstacle = EFloorObstacle::None;

	UPROPERTY()
	int randomFloorToSpawn = 0;

	UPROPERTY()
	int obstacleYaw = 0;
};

USTRUCT()
struct FSFloorObstacleByTexture
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	bool IsRandomYaw = false;

	UPROPERTY(EditAnywhere,meta = (EditCondition = "!IsRandomYaw",ClampMin = "0", UIMin = "0", ClampMax = "3", UIMax = "3"))
	int ObstacleYaw = 0;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class APackedLevelActor> ObstacleActor;

	UPROPERTY(EditAnywhere)
	EFloorObstacle FloorObstacle;
};

UCLASS()
class RELICRUNNERS_API ALevelGenerator : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALevelGenerator();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* Origin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USplineComponent* DeliverySpline;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values", meta = (ClampMin = "1", UIMin = "1", ClampMax = "1000", UIMax = "1000", ToolTip = "Distance tiles are from each other in unreal units"))
	float TileScale;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values")
	class AActor* CameraActor;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values")
	TSubclassOf<class ALevelChangeTrigger> LevelChangeTrigger;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values", meta = (ToolTip = "Level the players will travel to when in contact with the Level Change Trigger."))
	const TSoftObjectPtr<UWorld> TargetLevel;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values")
	bool GenerationIsRandom;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values", meta = (EditCondition = "!GenerationIsRandom", ToolTip = "If not nothing, level generation will place tiles based on position and colour of each pixel in the texture."))
	class UTexture2D* LevelTexture;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values", meta = (EditCondition = "!GenerationIsRandom", ToolTip = "If there are any specific modular pieces needed on a specific tile, define the colour and what spawns on top of every instance of that colour. Note: All non-white pixels in texture will have a floor set to that tile even if the specific colour isn't specified to have a modular piece."))
	TMap <FColor, FSFloorObstacleByTexture> TileColourToPackedActor;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values", meta = (EditCondition = "GenerationIsRandom", ClampMin = "0", UIMin = "0", ClampMax = "1000", UIMax = "1000", ToolTip = "Minimum distance the starting and ending tiles can be from each other."))
	int StartingToEndingMinDistance;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values", meta = (EditCondition = "GenerationIsRandom", ClampMin = "2", UIMin = "2", ClampMax = "100", UIMax = "100"))
	int SpawnWidth;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values", meta = (EditCondition = "GenerationIsRandom", ClampMin = "2", UIMin = "2", ClampMax = "100", UIMax = "100"))
	int SpawnDepth;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values", meta = (EditCondition = "GenerationIsRandom", ClampMin = "0", UIMin = "0", ClampMax = "100", UIMax = "100", ToolTip = "Percentage in which the tile is spawned walkable. 100% = All floor, 0% = No floor."))
	float FullPercentage;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values", meta = (EditCondition = "GenerationIsRandom", ClampMin = "0", UIMin = "0", ClampMax = "100", UIMax = "100", ToolTip = "Percentage in which the tile spawned is a hole. 100% = All holes, 0% = No holes. Note: All hole tiles will spawn a modular obstacle."))
	float HolePercentage;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values", meta = (EditCondition = "GenerationIsRandom", ClampMin = "0", UIMin = "0", ClampMax = "100", UIMax = "100", ToolTip = "Percentage in which a normal modular obstacle will be spawned on a walkable tile."))
	float BasicObstaclePercentage;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values", meta = (EditCondition = "GenerationIsRandom", ClampMin = "0", UIMin = "0", ClampMax = "100", UIMax = "100", ToolTip = "Size in which the center of the level is forced to spawn a walkable tile."))
	int CenterForceFull;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Values", meta = (EditCondition = "GenerationIsRandom", ClampMin = "0", UIMin = "0", ClampMax = "100", UIMax = "100", ToolTip = "Size in which the border of the level is forced to spawn a walkable tile."))
	int BorderForceFull;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Obstacle", meta = (ToolTip = "Array of regular packed level actors that the level generator will pick out of randomly to spawn."));
	TArray <TSubclassOf<class APackedLevelActor>> PackedActorArray;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Obstacle", meta = (ToolTip = "Array of walls associated with the regular packed level actor of the same index."));
	TArray <TSubclassOf<class APackedLevelActor>> PackedActorWallArray;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Obstacle", meta = (ToolTip = "Array of packed level actors specifically for hole tiles (full tiles without a floor)."));
	TArray <TSubclassOf<class APackedLevelActor>> HoleTilePackedActorArray;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Obstacle", meta = (ToolTip = "Array of walls associated with the packed level actor for hole tiles (full tiles without a floor) of the same index."));
	TArray <TSubclassOf<class APackedLevelActor>> HoleTilePackedActorWallArray;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Obstacle", meta = (ToolTip = "Packed level actor used as the starting point of the level."));
	TSubclassOf<class APackedLevelActor> LevelStartPackedActor;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Obstacle", meta = (ToolTip = "Packed level actor used as the ending point of the level."));
	TSubclassOf<class APackedLevelActor> LevelEndPackedActor;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Floor", meta = (ToolTip = "Mesh used for the quarter of a walkable tile with neighbours."))
	class UStaticMeshComponent* FullPiece;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Floor", meta = (ToolTip = "Mesh used for the quarter of a walkable tile without a horizontal OR vertical neighbour."))
	class UStaticMeshComponent* SidePiece;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Floor", meta = (ToolTip = "Mesh used for the quarter of an unwalkable tile with at least a horizontal and vertical neighbour."))
	class UStaticMeshComponent* ConcaveCornerPiece;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Modular Floor", meta = (ToolTip = "Mesh used for the quarter of a walkable tile without any neighbours."))
	class UStaticMeshComponent* ConvexCornerPiece;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenerationValues | Objective Values", meta = (Bitmask, BitmaskEnum = EObjectiveType))
	int ObjectiveType;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Objective Values| Capture the Flag", meta = (ToolTip = "Array of Packed level actor used as the *Capture the flag* objective points."));
	TArray<TSubclassOf<class APackedLevelActor>> CapturableFlagPackedActorArray;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Objective Values| Capture the Flag", meta = (ClampMin = "0", UIMin = "0", ClampMax = "1000", UIMax = "1000", ToolTip = "Maximum amount of tiles in which a flag objective is spawned on top of. Tile that are set as unwalkable are overriden to be walkable."))
	int MaxCapturableFlagAmount;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Objective Values| Enemies", meta = (ToolTip = "Array of Packed level actor used as the *Enemy Zone* objective points."));
	TArray<TSubclassOf<class APackedLevelActor>> EnemyZonePackedActorArray;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Objective Values| Enemies", meta = (ClampMin = "0", UIMin = "0", ClampMax = "1000", UIMax = "1000", ToolTip = "Maximum amount of tiles in which an enemy zone objective is spawned on top of. Tile that are set as unwalkable are overriden to be walkable."))
	int MaxEnemyZoneAmount;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Objective Values| Move the Payload")
	TSubclassOf<class APayloadActor> PayloadActor;

	UPROPERTY(EditAnywhere, Category = "GenerationValues | Objective Values| Move the Payload", meta = (ClampMin = "1", UIMin = "1", ToolTip = "Speed at which the payload moves towards the target."))
	float PayloadMovementSpeed;

protected:

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

public:
	//Used to call the functions that will initialize each tile individual and set it as a walkable or non-walkable tile.
	//Also used to set the regular modular obstacle that would spawn on it if it is set to walkable.
	void GenerateFloor(int x, int y);

	bool CalculateDistanceBetweenTiles(int firstIndex, int secondIndex);

	//Initializes the floor tile and adds it to the array of floor tiles
	void InitializeFloor();
	
	//If this is called, the tile's *IsFullTile* variable will be set to *forcedFloor* regardless of what *InitializeFloor()* set it to.
	void ForceFloorBool(bool forcedFloor, int x, int y, int width);

	//Sets wether or not the current tile will have a modular obstacle spawned on top of itself.
	void InitializeModularObstacle(FSFloorValues& floorValue, bool canGenerateHoles);

	//Finds the starting and ending indexes and sets them accordingly.
	void SetStartingAndEndingPoints(int startingIndex, int endingIndex);

	//Overrides tiles set by *InitializeFloor()* and sets them as Capturable Flag Tiles.
	void SetFlagTile(int index);

	//Overrides tiles set by *InitializeFloor()* and sets them as Enemy Zone Tiles.
	void SetEnemyZoneTile(int index);

	//Creates a path from *startingIndex* to *targetIndex* by setting those tile to walkable.
	void FindStartToEndPath(int startingIndex, int targetIndex, int width);
	void FindStartToEndPath(int startingIndex, int targetIndex, int width, bool implementSpline);

	//Checks the neighbours of each tile to determine what mesh should be used for each quarter tile.
	//Calls FloorCheckTopNeighbours, FloorCheckMiddleNeighbours and FloorCheckBottomNeighbours
	void CheckFloor(int x, int y, int width, int depth);

	void FloorCheckTopNeighbours(bool checkLeft, bool checkRight, int indexOffset, int x, int y, int width);
	void FloorCheckMiddleNeighbours(bool checkLeft, bool checkRight, int indexOffset, int x, int y, int width);
	void FloorCheckBottomNeighbours(bool checkLeft, bool checkRight, int indexOffset, int x, int y, int width);

	//Sets each tile's shape based on neighbours.
	//Calls SetTopLeftFloorShape, SetTopRightFloorShape, SetBottomLeftFloorShape and SetBottomRightFloorShape.
	void SetFloorShape(int x, int y, int width);

	void SetTopLeftFloorShape(int x, int y, int width);
	void SetTopRightFloorShape(int x, int y, int width);
	void SetBottomLeftFloorShape(int x, int y, int width);
	void SetBottomRightFloorShape(int x, int y, int width);

	//Spawns the modular obstacles based on previously defined variable.
	UFUNCTION(Server, Reliable)
	void Server_SpawnFloorObstacles(int x, int y, int width);
	void SpawnFloorObstacles(int x, int y, int width);

	//Spawn modular obstacles based on the colours set if spawning a level with a texture.
	UFUNCTION(Server, Reliable)
	void Server_SpawnFloorObstaclesByColour(int x, int y, int width, FColor colour);
	void SpawnFloorObstaclesByColour(int x, int y, int width, FColor colour);

	//Spawns the appropriate mesh into an instanced static mesh component based on the tile's neighbours.
	void CreateFloor();


	int GetMapWidth() { return SpawnWidth; }
	int GetMapDepth() { return SpawnDepth; }

	float GetFullPercentage() { return FullPercentage; }

public:
	//Client only. Used to replicate the array of floor values.
	UFUNCTION()
	void OnRep_FloorValuesArrayChange();

	UPROPERTY(ReplicatedUsing = OnRep_FloorValuesArrayChange)
	TArray<FSFloorValues> FloorValuesArray;

	TArray<FTransform> FullFloorPieceTransforms;
	TArray<FTransform> SideFloorPieceTransforms;
	TArray<FTransform> ConcaveCornerPieceTransforms;
	TArray<FTransform> ConvexCornerTransforms;

	bool bHasSpawnedStartingPoint;
	bool bHasSpawnerEndingPoint;
	int CurrentlySpawnedBasicObstacles;
	int CurrentlySpawnedEnemyTowers;
	int CurrentlySpawnedShops;
};
