// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IslandGenerationTest.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnObjectiveIslandObjectiveComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSetCameraChange, int, Index);

enum class EIslandObjectiveType : uint8;

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EIslandFloorNeighbours : uint8
{
	Blank = 0,
	TopLeft = 1 << 0,		/*0000 0001*/
	TopMiddle = 1 << 1,		/*0000 0010*/
	TopRight = 1 << 2,		/*0000 0100*/
	MiddleLeft = 1 << 3,		/*0000 1000*/
	MiddleRight = 1 << 4,		/*0001 0000*/
	BottomLeft = 1 << 5,		/*0010 0000*/
	BottomMiddle = 1 << 6,		/*0100 0000*/
	BottomRight = 1 << 7,		/*1000 0000*/
};
ENUM_CLASS_FLAGS(EIslandFloorNeighbours);

UENUM(BlueprintType)
enum class EIslandFloorObstacle : uint8
{
	None,
	Basic,
	Hole,
	Vantage,
	CapturableFlag,
	EnemyZone,
	DeliveryPath,
	SurvivalZone,
	Start,
	End,
};


USTRUCT()
struct FSIslandFloorValues
{
	GENERATED_BODY()
public:
	UPROPERTY()
	bool IsFullTile = false;

	UPROPERTY()
	EIslandFloorNeighbours FloorNeighbours = EIslandFloorNeighbours::Blank;

	UPROPERTY()
	EIslandFloorNeighbours ObstacleNeighbours = EIslandFloorNeighbours::Blank;

	UPROPERTY()
	EIslandFloorObstacle IslandFloorObstacle = EIslandFloorObstacle::None;

	UPROPERTY()
	uint8 randomFloorToSpawn = 0;

	UPROPERTY()
	int obstacleYaw = 0;
};

UCLASS()
class RELICRUNNERS_API AIslandGenerationTest : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIslandGenerationTest();

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UIslandSpawningValuesComponent* GenerationValuesComponent;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USplineComponent* SplinePath;

	void InitializeValues(EIslandObjectiveType islandObjective);

	void Generate();

	void ActivateIsland();

	//Used to call the functions that will initialize each tile individually and set it as a walkable or non-walkable tile.
	//Also used to set the regular modular obstacle that would spawn on it if it is set to walkable.
	void GenerateFloor(int x, int y);

	bool CalculateDistanceBetweenTiles(int firstIndex, int secondIndex) const;

	//Initializes the floor tile and adds it to the array of floor tiles
	void InitializeFloor();

	//If this is called, the tile's *IsFullTile* variable will be set to *forcedFloor* regardless of what *InitializeFloor()* set it to.
	void ForceFloorBool(bool forcedFloor, int x, int y, int width, bool canSpawnObstacle);

	//Sets wether or not the current tile will have a modular obstacle spawned on top of itself.
	void InitializeModularObstacle(FSIslandFloorValues& floorValue, bool canGenerateHoles) const;

	//Finds the starting and ending indexes and sets them accordingly.
	void SetStartingAndEndingPoints(int startingIndex, int endingIndex);

	//Overrides tiles set by *InitializeFloor()* and sets them as Capturable Flag Tiles.
	void SetFlagTile(int index);

	////Overrides tiles set by *InitializeFloor()* and sets them as Enemy Zone Tiles.
	void SetEnemyZoneTile(int index);

	////Overrides tiles set by *InitializeFloor()* and sets them as Survival Zone Tiles.
	void SetSurvivalZoneTile(int index);

	////Overrides tiles set by *InitializeFloor()* and sets them as Vantage Point Tiles.
	void SetVantagePointTile(int index);

	//Creates a path from *startingIndex* to *targetIndex* by setting those tiles to walkable.
	void FindStartToEndPath(int startingIndex, int targetIndex, int width);

	void CreateSplinePathStartToEnd(int startingIndex, int targetIndex, int width, class ANewPayloadObjective* payloadActor);

	void AddFlagObjectiveToAll(int NumFlags);

	void DecrementFlagObjectiveForAll();

	void AddEnemiesObjectiveToAll(int TotalEnemies);

	void AddPandoraObjectiveToAll(float Time);

	void AddPayloadObjectiveToAll();

	//Initializes the objective
	void InitializeObjectives(int startingIndex, int endingIndex);

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

	//Sets each tile's shape based on neighbours.
	//Calls SetTopLeftFloorShape, SetTopRightFloorShape, SetBottomLeftFloorShape and SetBottomRightFloorShape.
	void SetWall(int tileIndex, FVector posOffset, TSubclassOf<class APackedLevelActor> CornerWall, TSubclassOf<class APackedLevelActor> SideWall);

	void SetTopLeftWall(int tileIndex, FVector posOffset, TSubclassOf<class APackedLevelActor> CornerWall, TSubclassOf<class APackedLevelActor> SideWall);
	void SetTopRightWall(int tileIndex, FVector posOffset, TSubclassOf<class APackedLevelActor> CornerWall, TSubclassOf<class APackedLevelActor> SideWall);
	void SetBottomLeftWall(int tileIndex, FVector posOffset, TSubclassOf<class APackedLevelActor> CornerWall, TSubclassOf<class APackedLevelActor> SideWall);
	void SetBottomRightWall(int tileIndex, FVector posOffset, TSubclassOf<class APackedLevelActor> CornerWall, TSubclassOf<class APackedLevelActor> SideWall);

	//Spawns the modular obstacles based on previously defined variable.
	UFUNCTION(Server, Reliable)
	void Server_SpawnFloorObstacles(int x, int y, int width);
	void SpawnFloorObstacles(int x, int y, int width);

	//Spawns the appropriate mesh into an instanced static mesh component based on the tile's neighbours.
	void CreateFloor();

	UFUNCTION()
	void IncrementObjective();
	UFUNCTION()
	void DecrementObjective();

	void SetFinalIsland(bool isFinal) { isFinalIsland = isFinal; }

	UFUNCTION()
	void UpdateCameraPos();

	UFUNCTION()
	void SetNextCameraPos();

public:
	//Client only. Used to replicate the array of floor values.
	UFUNCTION()
	void OnRep_FloorValuesArrayChange();

	UPROPERTY(ReplicatedUsing = OnRep_FloorValuesArrayChange)
	TArray<FSIslandFloorValues> FloorValuesArray;

	TArray<FTransform> FullFloorPieceTransforms;
	TArray<FTransform> SideFloorPieceTransforms;
	TArray<FTransform> ConvexCornerTransforms;

	UPROPERTY(Replicated)
	uint8 IslandWidth;
	UPROPERTY(Replicated)
	uint8 IslandDepth;

	uint8 IslandIndex;

	FOnObjectiveIslandObjectiveComplete OnObjectiveIslandObjectiveComplete;
	FOnSetCameraChange OnSetCameraChange;

	int8 CurrentObjectives;

	EIslandObjectiveType IslandObjective;

	class AIslandBridge* Bridge;

	bool isFinalIsland;

	int ObjectiveAmount;

	class AActor* CameraActor;

	UPROPERTY(EditAnywhere, Category = "Default Values | Enemy Spawning Zone Actor");
	TSubclassOf<class ASpawnZone> SpawnZoneTemplate;

	TArray<class AObjectiveBase*> ObjectiveArrays;
};
