// Fill out your copyright notice in the Description page of Project Settings.


#include "IslandGenerationTest.h"
#include "Net/UnrealNetwork.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "IslandSpawningValuesComponent.h"
#include "Net/UnrealNetwork.h"
#include "PackedLevelActor/PackedLevelActor.h"
#include "NewObjectives/NewCapturableFlag.h"
#include "NewObjectives/NewEnemyZone.h"
#include "NewObjectives/NewPayloadObjective.h"
#include "NewObjectives/NewSurvivalZone.h"
#include "IslandBridge.h"
#include "LevelChangeTrigger.h"
#include "Components/SplineComponent.h"
#include "NavigationSystem.h"
#include "RelicRunners/Director System/SpawnZone.h"

/*
Set index of starting tile and ending (bridge) tile

Initialize values of each floor tile based on width and depth + set what regular obstacle will spawn based on a precentage

Override previously defined tiles with starting and ending tiles

Based on maximum amount, override previously defined tiles (excluding the starting and ending tiles) with the objective of that island
Spawn the objective at the appropriate location

Spawn regular obstacles at their appropriate location + Spawn their appropriate walls with instant static mesh

*/

/*TODO

Fix the wall spawning | Done

Each objective have their own max and min width and depth | Done

Setup Dynamic navmesh in random level | Done

Fix Camera in random level | Done

Add flag and pandora's box meshes | Done

Handle different difficulty

*/

// Sets default values
AIslandGenerationTest::AIslandGenerationTest() :
	IslandWidth(2),
	IslandDepth(2),
	IslandIndex(0),
	CurrentObjectives(0),
	isFinalIsland(false),
	ObjectiveAmount(0)
{
	PrimaryActorTick.bCanEverTick = false;

	GenerationValuesComponent = CreateDefaultSubobject<UIslandSpawningValuesComponent>("GenerationValues");
	GenerationValuesComponent->SetIsReplicated(true);

	SplinePath = CreateDefaultSubobject<USplineComponent>("SplinePath");


	bReplicates = true;
	bAlwaysRelevant = true;
}

void AIslandGenerationTest::Generate()
{
	if (HasAuthority())
	{
		//Set Camera Position relative to size of map
		if (CameraActor)
		{
			float tileScale = GenerationValuesComponent->IslandValues.TileScale;
			float cameraX = IslandWidth * tileScale * 2 - tileScale;
			float cameraY = IslandDepth * tileScale * 2 - tileScale;
			CameraActor->SetActorLocation(FVector(cameraX+GetActorLocation().X, cameraY+GetActorLocation().Y, 0.0f));
		}


		//Initialize the starting and ending tiles.
		int startingIndex = 0, endingIndex = (IslandDepth * IslandWidth) - 1;

		//If somehow both are at the exact same index, reinitialize starting and ending tiles
		//do
		//{
		//	startingIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

		//} while (CalculateDistanceBetweenTiles(startingIndex, endingIndex) || startingIndex == endingIndex);

		for (int y = 0; y < IslandDepth; y++)
		{
			for (int x = 0; x < IslandWidth; x++)
			{
				//Initialize each tile and set them to either walkable or non-walkable.
				GenerateFloor(x, y);
			}
		}

		if (IslandDepth * IslandWidth > 1)
		{
			//Override starting and ending tiles initialized above and create a path from one to the other.
			SetStartingAndEndingPoints(startingIndex, endingIndex);

			FindStartToEndPath(startingIndex, endingIndex, IslandWidth);
		}

		InitializeObjectives(startingIndex, endingIndex);

		int maxVantagePoint = FMath::RandRange(GenerationValuesComponent->VantageValues.MaxVantageAmountMin, GenerationValuesComponent->VantageValues.MaxVantageAmountMax);

		//Override previously initialized tiles with Vantage Point tiles
		for (int i = 0; i < maxVantagePoint; i++)
		{
			int keyTileIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

			int whileLoopLimit = 0;

			//If the currently selected tile can be overriden, do that, if not, find another tile. Repeat 10 time. if failed every time, give up.
			while (FloorValuesArray[keyTileIndex].IslandFloorObstacle > EIslandFloorObstacle::Vantage)
			{
				keyTileIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

				whileLoopLimit++;

				if (whileLoopLimit >= 10)
				{
					break;
				}
			}

			//If it can, override the currently selected tile with the key tile and create a path from the starting tile to it.
			if (whileLoopLimit < 10)
			{
				SetVantagePointTile(keyTileIndex);

				FindStartToEndPath(startingIndex, keyTileIndex, IslandWidth);
			}
		}

		if (!FloorValuesArray.IsEmpty())
		{
			for (int y = 0; y < IslandDepth; y++)
			{
				for (int x = 0; x < IslandWidth; x++)
				{
					//Spawn the floor tiles with the instanced static mesh component.
					CheckFloor(x, y, IslandWidth, IslandDepth);

					//Spawn each modular obstacle based on previously defined variable.
					Server_SpawnFloorObstacles(x, y, IslandWidth);
				}
			}
			//Spawn the floor with the instanced static mesh component.
			CreateFloor();
		}

		if (GenerationValuesComponent->IslandStartEnd.BridgePackedActor)
		{
			float tileScale = GenerationValuesComponent->IslandValues.TileScale;
			FVector posOffset = FVector((tileScale * IslandWidth * 4) + tileScale, (tileScale * (IslandDepth - 1) * 4) + tileScale, 0.0f) + GetActorLocation();

			Bridge = GetWorld()->SpawnActor<AIslandBridge>(GenerationValuesComponent->IslandStartEnd.BridgePackedActor, posOffset, FRotator::ZeroRotator);

			Bridge->SetFinalBridge(isFinalIsland);

			if (GenerationValuesComponent->IslandValues.LevelChangeTrigger && isFinalIsland)
			{
				ALevelChangeTrigger* levelTrigger = GetWorld()->SpawnActor<ALevelChangeTrigger>(GenerationValuesComponent->IslandValues.LevelChangeTrigger, posOffset, FRotator::ZeroRotator);
				levelTrigger->Server_SetTargetLevel(FName(GenerationValuesComponent->IslandValues.TargetLevel));
			}
		}
	}

	//Navmesh stuff. Thanks Konstantin
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		NavSys->Build();
	}

	if (SpawnZoneTemplate == nullptr || IslandObjective == EIslandObjectiveType::DefeatAllEnemies)
		return;
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;
	FVector enemyZoneOffset = FVector((tileScale * IslandWidth * 4) / 2, (tileScale * (IslandDepth) * 4) / 2, 0.0f);

	ASpawnZone* spawnZone = GetWorld()->SpawnActor<ASpawnZone>(SpawnZoneTemplate, enemyZoneOffset+GetActorLocation(), GetActorRotation());
}

void AIslandGenerationTest::InitializeValues(EIslandObjectiveType islandObjective)
{
	IslandObjective = islandObjective;
	UIslandSpawningValuesComponent* ValuesComp = GenerationValuesComponent;

	switch (IslandObjective)
	{
	case EIslandObjectiveType::CaptureTheFlag:
		IslandDepth = FMath::RandRange(ValuesComp->FlagObjectiveValues.FlagDepthMin, ValuesComp->FlagObjectiveValues.FlagDepthMax);
		IslandWidth = FMath::RandRange(ValuesComp->FlagObjectiveValues.FlagWidthMin, ValuesComp->FlagObjectiveValues.FlagWidthMax);
		break;
	case EIslandObjectiveType::DefeatAllEnemies:
		IslandDepth = FMath::RandRange(ValuesComp->EnemyZoneObjectiveValues.EnemyZoneDepthMin, ValuesComp->EnemyZoneObjectiveValues.EnemyZoneDepthMax);
		IslandWidth = FMath::RandRange(ValuesComp->EnemyZoneObjectiveValues.EnemyZoneWidthMin, ValuesComp->EnemyZoneObjectiveValues.EnemyZoneWidthMax);
		break;
	case EIslandObjectiveType::MoveThePayload:
		IslandDepth = FMath::RandRange(ValuesComp->PayloadObjectiveValues.PayloadDepthMin, ValuesComp->PayloadObjectiveValues.PayloadDepthMax);
		IslandWidth = FMath::RandRange(ValuesComp->PayloadObjectiveValues.PayloadWidthMin, ValuesComp->PayloadObjectiveValues.PayloadWidthMax);
		break;
	case EIslandObjectiveType::Survival:
		IslandDepth = FMath::RandRange(ValuesComp->SurvivalZoneObjectiveValues.SurvivalDepthMin, ValuesComp->SurvivalZoneObjectiveValues.SurvivalDepthMax);
		IslandWidth = FMath::RandRange(ValuesComp->SurvivalZoneObjectiveValues.SurvivalWidthMin, ValuesComp->SurvivalZoneObjectiveValues.SurvivalWidthMax);
		break;
	default:
		IslandDepth = 2;
		IslandWidth = 2;
		break;
	}
}

void AIslandGenerationTest::GenerateFloor(int x, int y)
{
	if (FloorValuesArray.Num() == x + (IslandWidth * y))
	{
		//Set current tile as walkable or non-walkable
		InitializeFloor();
	}
}

bool AIslandGenerationTest::CalculateDistanceBetweenTiles(int firstIndex, int secondIndex) const
{
	uint8 minDistance = GenerationValuesComponent->IslandStartEnd.StartingToEndingMinDistance;
	if ((minDistance + (minDistance * IslandWidth)) > IslandWidth * IslandDepth)
	{
		return false;
	}

	for (int y = -minDistance; y < minDistance; y++)
	{
		for (int x = -minDistance; x < minDistance; x++)
		{
			int currentX = firstIndex % IslandWidth + x;
			int currentY = firstIndex / IslandWidth + y;

			bool validX = currentX >= 0 && currentX < IslandWidth;
			bool validY = currentY >= 0 && currentY < IslandDepth;

			if (validX && validY)
			{
				if ((currentX + (currentY * IslandWidth)) == secondIndex)
				{
					return true;
				}
			}
		}
	}
	return false;
}

void AIslandGenerationTest::SetStartingAndEndingPoints(int startingIndex, int endingIndex)
{
	FloorValuesArray[startingIndex].IsFullTile = true;
	FloorValuesArray[startingIndex].IslandFloorObstacle = EIslandFloorObstacle::Start;
	FloorValuesArray[startingIndex].randomFloorToSpawn = 0;
	FloorValuesArray[startingIndex].obstacleYaw = FMath::RandRange(0, 3) * 90;


	FloorValuesArray[endingIndex].IsFullTile = true;
	FloorValuesArray[endingIndex].IslandFloorObstacle = EIslandFloorObstacle::End;
	FloorValuesArray[endingIndex].randomFloorToSpawn = 0;
	FloorValuesArray[endingIndex].obstacleYaw = FMath::RandRange(0, 3) * 90;
}

void AIslandGenerationTest::SetFlagTile(int index)
{
	FloorValuesArray[index].IsFullTile = true;
	FloorValuesArray[index].IslandFloorObstacle = EIslandFloorObstacle::CapturableFlag;
	FloorValuesArray[index].randomFloorToSpawn = 0;
	FloorValuesArray[index].obstacleYaw = FMath::RandRange(0, 3) * 90;
}

void AIslandGenerationTest::SetEnemyZoneTile(int index)
{
	FloorValuesArray[index].IsFullTile = true;
	FloorValuesArray[index].IslandFloorObstacle = EIslandFloorObstacle::EnemyZone;
	FloorValuesArray[index].randomFloorToSpawn = 0;
	FloorValuesArray[index].obstacleYaw = FMath::RandRange(0, 3) * 90;
}

void AIslandGenerationTest::SetSurvivalZoneTile(int index)
{
	FloorValuesArray[index].IsFullTile = true;
	FloorValuesArray[index].IslandFloorObstacle = EIslandFloorObstacle::SurvivalZone;
	FloorValuesArray[index].randomFloorToSpawn = 0;
	FloorValuesArray[index].obstacleYaw = FMath::RandRange(0, 3) * 90;
}

void AIslandGenerationTest::SetVantagePointTile(int index)
{
	FloorValuesArray[index].IsFullTile = true;
	FloorValuesArray[index].IslandFloorObstacle = EIslandFloorObstacle::Vantage;
	FloorValuesArray[index].randomFloorToSpawn = 0;
	FloorValuesArray[index].obstacleYaw = FMath::RandRange(0, 3) * 90;
}

void AIslandGenerationTest::InitializeFloor()
{	
	//Based on the percentage, set floor tile to walkable or non-walkable.
	FSIslandFloorValues newFloorValues;
	float randomFloor = FMath::RandRange(0.1f, 100.0f);

	float fullPercentage = GenerationValuesComponent->IslandValues.FullPercentage;

	if (randomFloor > fullPercentage)
	{
		newFloorValues.IsFullTile = false;
	}
	else
	{
		newFloorValues.IsFullTile = true;

		InitializeModularObstacle(newFloorValues, true);
	}

	//Add initialized floor to array
	FloorValuesArray.Add(newFloorValues);
}

void AIslandGenerationTest::ForceFloorBool(bool forcedFloor, int x, int y, int width, bool canSpawnObstacle)
{
	int index = x + (y * width);

	if (EnumHasAnyFlags(FloorValuesArray[index].IslandFloorObstacle, EIslandFloorObstacle::Hole))
		return;

	FloorValuesArray[index].IsFullTile = forcedFloor;

	if (!forcedFloor)
		return;

	if (FloorValuesArray[index].IslandFloorObstacle == EIslandFloorObstacle::None)
		return;

	if (FloorValuesArray[index].IslandFloorObstacle >= EIslandFloorObstacle::Start)
		return;

	if (!canSpawnObstacle)
	{
		FloorValuesArray[index].IslandFloorObstacle = EIslandFloorObstacle::None;
		return;
	}

	InitializeModularObstacle(FloorValuesArray[index], false);
}

void AIslandGenerationTest::InitializeModularObstacle(FSIslandFloorValues& floorValue, bool canGenerateHoles) const
{
	float holeTilePercentage = FMath::RandRange(0.1f, 100.0f);

	if (holeTilePercentage < GenerationValuesComponent->IslandValues.HolePercentage)
	{
		if (canGenerateHoles)
		{
			floorValue.IslandFloorObstacle = EIslandFloorObstacle::Hole;

			floorValue.randomFloorToSpawn = FMath::RandRange(0, GenerationValuesComponent->HoleValues.HoleTileActor.Num() - 1);

			floorValue.obstacleYaw = FMath::RandRange(0, 3) * 90;

			floorValue.IsFullTile = false;
		}
		else
		{
			float randomObstaclePerc = FMath::RandRange(0.1f, 100.0f);
			if (randomObstaclePerc < GenerationValuesComponent->ObstacleValues.ObstaclePercentage)
			{
				floorValue.IslandFloorObstacle = EIslandFloorObstacle::Basic;

				floorValue.randomFloorToSpawn = FMath::RandRange(0, GenerationValuesComponent->ObstacleValues.ObstacleActor.Num() - 1);

				floorValue.obstacleYaw = FMath::RandRange(0, 3) * 90;
			}
			else
			{
				floorValue.IslandFloorObstacle = EIslandFloorObstacle::None;
			}
		}
	}
	else
	{
		float randomObstaclePerc = FMath::RandRange(0.1f, 100.0f);
		if (randomObstaclePerc < GenerationValuesComponent->ObstacleValues.ObstaclePercentage)
		{
			floorValue.IslandFloorObstacle = EIslandFloorObstacle::Basic;

			floorValue.randomFloorToSpawn = FMath::RandRange(0, GenerationValuesComponent->ObstacleValues.ObstacleActor.Num() - 1);

			floorValue.obstacleYaw = FMath::RandRange(0, 3) * 90;
		}
	}
}

void AIslandGenerationTest::FindStartToEndPath(int startingIndex, int targetIndex, int width)
{
	int currentX = startingIndex % width;
	int currentY = startingIndex / width;

	int targetX = targetIndex % width;
	int targetY = targetIndex / width;

	bool bReachEndX = currentX == targetX;

	bool bReachEndY = currentY == targetY;

	//To avoid repetition, randomly picks between moving horizontally or vertically closer to the target
	while (!(bReachEndX && bReachEndY))
	{
		float random = FMath::FRandRange(0.0f, 100.0f);

		if (random >= 50.0f)
		{
			if (currentX > targetX)
			{
				currentX--;
			}
			else if (currentX < targetX)
			{
				currentX++;
			}
		}
		else
		{
			if (currentY > targetY)
			{
				currentY--;
			}
			else if (currentY < targetY)
			{
				currentY++;
			}
		}

		bReachEndX = currentX == targetX;
		bReachEndY = currentY == targetY;
		if (!(bReachEndX && bReachEndY))
		{
			ForceFloorBool(true, currentX, currentY, width, true);
		}
	}
}

void AIslandGenerationTest::CreateSplinePathStartToEnd(int startingIndex, int targetIndex, int width, ANewPayloadObjective* payloadActor)
{
	int currentX = startingIndex % width;
	int currentY = startingIndex / width;

	int targetX = targetIndex % width;
	int targetY = targetIndex / width;

	bool bReachEndX = currentX == targetX;

	bool bReachEndY = currentY == targetY;

	bool bValueHasChanged = false;
	//To avoid repetition, randomly picks between moving horizontally or vertically closer to the target
	while (!(bReachEndX && bReachEndY))
	{
		float random = FMath::FRandRange(0.0f, 100.0f);

		if (random >= 50.0f)
		{
			if (currentX > targetX)
			{
				currentX--;
				bValueHasChanged = true;
			}
			else if (currentX < targetX)
			{
				currentX++;
				bValueHasChanged = true;
			}
		}
		else
		{
			if (currentY > targetY)
			{
				currentY--;
				bValueHasChanged = true;
			}
			else if (currentY < targetY)
			{
				currentY++;
				bValueHasChanged = true;
			}
		}

		if (!(bReachEndX && bReachEndY))
		{
			ForceFloorBool(true, currentX, currentY, width, false);

			if (payloadActor && bValueHasChanged)
			{
				float tileScale = GenerationValuesComponent->IslandValues.TileScale;
				FVector pointPos = FVector((tileScale * currentX * 4) + tileScale, (tileScale * currentY * 4) + tileScale, 0.0f) + GetActorLocation();

				SplinePath->AddSplinePoint(pointPos, ESplineCoordinateSpace::World, true);

				bValueHasChanged = false;
			}
		}
		bReachEndX = currentX == targetX;
		bReachEndY = currentY == targetY;
	}
}

void AIslandGenerationTest::InitializeObjectives(int startingIndex, int endingIndex)
{
	int flagAmount = FMath::RandRange(GenerationValuesComponent->FlagObjectiveValues.FlagAmountMin, GenerationValuesComponent->FlagObjectiveValues.FlagAmountMax);
	int enemyZoneAmount = 1;
	int survivalZoneAmount = 1;

	switch (IslandObjective)
	{
	case EIslandObjectiveType::CaptureTheFlag:
		ObjectiveAmount = flagAmount;

		//Override previously initialized tiles with capturable flag tiles
		for (int i = 0; i < ObjectiveAmount; i++)
		{
			int keyTileIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

			int whileLoopLimit = 0;

			//If the currently selected tile can be overriden, do that, if not, find another tile. Repeat 10 time. if failed every time, give up.
			while (FloorValuesArray[keyTileIndex].IslandFloorObstacle > EIslandFloorObstacle::Basic)
			{
				keyTileIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

				whileLoopLimit++;

				if (whileLoopLimit >= 10)
				{
					break;
				}
			}

			//If it can, override the currently selected tile with the key tile and create a path from the starting tile to it.
			if (whileLoopLimit < 10)
			{
				SetFlagTile(keyTileIndex);

				FindStartToEndPath(startingIndex, keyTileIndex, IslandWidth);
			}
		}
		break;
	case EIslandObjectiveType::DefeatAllEnemies:
		ObjectiveAmount = enemyZoneAmount;

		for (int i = 0; i < ObjectiveAmount; i++)
		{
			int keyTileIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

			int whileLoopLimit = 0;

			//If the currently selected tile can be overriden, do that, if not, find another tile. Repeat 10 time. if failed every time, give up.
			while (FloorValuesArray[keyTileIndex].IslandFloorObstacle > EIslandFloorObstacle::Basic)
			{
				keyTileIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

				whileLoopLimit++;

				if (whileLoopLimit >= 10)
				{
					break;
				}
			}

			//If it can, override the currently selected tile with the key tile and create a path from the starting tile to it.
			if (whileLoopLimit < 10)
			{
				SetEnemyZoneTile(keyTileIndex);

				FindStartToEndPath(startingIndex, keyTileIndex, IslandWidth);
			}
		}
		break;
	case EIslandObjectiveType::MoveThePayload:
		if (IslandObjective == EIslandObjectiveType::MoveThePayload)
		{
			if (IslandDepth * IslandWidth > 1)
			{
				if (GenerationValuesComponent->PayloadObjectiveValues.Payload != nullptr)
				{
					SplinePath->SetLocationAtSplinePoint(0, GetActorLocation(), ESplineCoordinateSpace::World, true);

					float tileScale = GenerationValuesComponent->IslandValues.TileScale;
					int x = startingIndex % IslandWidth;
					int y = startingIndex / IslandWidth;

					FVector posOffset = FVector((tileScale * x * 4) + tileScale, (tileScale * y * 4) + tileScale, 0.0f) + GetActorLocation();

					ANewPayloadObjective* payload = GetWorld()->SpawnActor<ANewPayloadObjective>(GenerationValuesComponent->PayloadObjectiveValues.Payload, posOffset, FRotator::ZeroRotator);
					CreateSplinePathStartToEnd(startingIndex, endingIndex, IslandWidth, payload);

					FObjectivePayloadValues val = GenerationValuesComponent->PayloadObjectiveValues;

					payload->Initialize(val.PayloadSpeed, val.Acceleration, val.Deceleration, SplinePath);

					payload->OnObjectiveCreated.AddDynamic(this, &AIslandGenerationTest::IncrementObjective);
					payload->OnObjectiveActivated.AddDynamic(this, &AIslandGenerationTest::DecrementObjective);
					payload->Activate();
				}
			}
		}

		break;
	case EIslandObjectiveType::Survival:
		ObjectiveAmount = survivalZoneAmount;

		//Override previously initialized tiles with survival zone tiles
		for (int i = 0; i < ObjectiveAmount; i++)
		{
			int keyTileIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

			int whileLoopLimit = 0;

			//If the currently selected tile can be overriden, do that, if not, find another tile. Repeat 10 time. if failed every time, give up.
			while (FloorValuesArray[keyTileIndex].IslandFloorObstacle > EIslandFloorObstacle::Basic)
			{
				keyTileIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

				whileLoopLimit++;

				if (whileLoopLimit >= 10)
				{
					break;
				}
			}

			//If it can, override the currently selected tile with the key tile and create a path from the starting tile to it.
			if (whileLoopLimit < 10)
			{
				SetSurvivalZoneTile(keyTileIndex);

				FindStartToEndPath(startingIndex, keyTileIndex, IslandWidth);
			}
		}
		break;
	}
}

void AIslandGenerationTest::CheckFloor(int x, int y, int width, int depth)
{
	bool checkLeft = x > 0;
	bool checkRight = x < width - 1;

	bool checkUp = y > 0;
	bool checkDown = y < depth - 1;

	if (checkUp)
	{
		FloorCheckTopNeighbours(checkLeft, checkRight, -width, x, y, width);
	}

	FloorCheckMiddleNeighbours(checkLeft, checkRight, 0, x, y, width);

	if (checkDown)
	{
		FloorCheckBottomNeighbours(checkLeft, checkRight, width, x, y, width);
	}

	SetFloorShape(x, y, width);
}

void AIslandGenerationTest::FloorCheckTopNeighbours(bool checkLeft, bool checkRight, int indexOffset, int x, int y, int width)
{
	int indexToCheck = x + (y * width) + indexOffset;
	int index = x + (y * width);

	if (checkLeft)
	{
		if (FloorValuesArray[indexToCheck - 1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EIslandFloorNeighbours::TopLeft;
			FloorValuesArray[index].ObstacleNeighbours |= EIslandFloorNeighbours::TopLeft;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EIslandFloorNeighbours::TopLeft;
		}
	}

	if (FloorValuesArray[indexToCheck].IsFullTile)
	{
		FloorValuesArray[index].FloorNeighbours |= EIslandFloorNeighbours::TopMiddle;
		FloorValuesArray[index].ObstacleNeighbours |= EIslandFloorNeighbours::TopMiddle;
	}
	else
	{
		FloorValuesArray[index].FloorNeighbours &= ~EIslandFloorNeighbours::TopMiddle;
	}

	if (checkRight)
	{
		if (FloorValuesArray[indexToCheck + 1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EIslandFloorNeighbours::TopRight;
			FloorValuesArray[index].ObstacleNeighbours |= EIslandFloorNeighbours::TopRight;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EIslandFloorNeighbours::TopRight;
		}
	}
}

void AIslandGenerationTest::FloorCheckMiddleNeighbours(bool checkLeft, bool checkRight, int indexOffset, int x, int y, int width)
{
	int indexToCheck = x + (y * width) + indexOffset;
	int index = x + (y * width);

	if (checkLeft)
	{
		if (FloorValuesArray[indexToCheck - 1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EIslandFloorNeighbours::MiddleLeft;
			FloorValuesArray[index].ObstacleNeighbours |= EIslandFloorNeighbours::MiddleLeft;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EIslandFloorNeighbours::MiddleLeft;
		}
	}

	if (checkRight)
	{
		if (FloorValuesArray[indexToCheck + 1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EIslandFloorNeighbours::MiddleRight;
			FloorValuesArray[index].ObstacleNeighbours |= EIslandFloorNeighbours::MiddleRight;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EIslandFloorNeighbours::MiddleRight;
		}
	}
}

void AIslandGenerationTest::FloorCheckBottomNeighbours(bool checkLeft, bool checkRight, int indexOffset, int x, int y, int width)
{
	int indexToCheck = x + (y * width) + indexOffset;
	int index = x + (y * width);

	if (checkLeft)
	{
		if (FloorValuesArray[indexToCheck - 1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EIslandFloorNeighbours::BottomLeft;
			FloorValuesArray[index].ObstacleNeighbours |= EIslandFloorNeighbours::BottomLeft;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EIslandFloorNeighbours::BottomLeft;
		}
	}

	if (FloorValuesArray[indexToCheck].IsFullTile)
	{
		FloorValuesArray[index].FloorNeighbours |= EIslandFloorNeighbours::BottomMiddle;
		FloorValuesArray[index].ObstacleNeighbours |= EIslandFloorNeighbours::BottomMiddle;
	}
	else
	{
		FloorValuesArray[index].FloorNeighbours &= ~EIslandFloorNeighbours::BottomMiddle;
	}

	if (checkRight)
	{
		if (FloorValuesArray[indexToCheck + 1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EIslandFloorNeighbours::BottomRight;
			FloorValuesArray[index].ObstacleNeighbours |= EIslandFloorNeighbours::BottomRight;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EIslandFloorNeighbours::BottomRight;
		}
	}
}

void AIslandGenerationTest::SetFloorShape(int x, int y, int width)
{
	SetTopLeftFloorShape(x, y, width);
	SetTopRightFloorShape(x, y, width);
	SetBottomLeftFloorShape(x, y, width);
	SetBottomRightFloorShape(x, y, width);
}

void AIslandGenerationTest::SetTopLeftFloorShape(int x, int y, int width)
{
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	int index = x + (y * width);
	FVector posOffset = FVector((x * 2), (y * 2), 0.0f) * (tileScale * 2) + GetActorLocation();
	FRotator rotationOffset = FRotator(0.0f, 0.0f, 0.0f);
	float scaleOffset = tileScale * 2;
	FTransform newTransform;

	newTransform.SetLocation(posOffset);
	newTransform.SetRotation(rotationOffset.Quaternion());
	newTransform.SetScale3D(FVector(scaleOffset));

	EIslandFloorNeighbours topLeftCornerCheck = FloorValuesArray[index].FloorNeighbours & (EIslandFloorNeighbours::TopLeft | EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleLeft);

	if (FloorValuesArray[index].IsFullTile)
	{
		switch (topLeftCornerCheck)
		{
		case EIslandFloorNeighbours::TopMiddle:
		case EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::TopLeft:
			SideFloorPieceTransforms.Add(newTransform);
			break;
		case EIslandFloorNeighbours::MiddleLeft:
		case EIslandFloorNeighbours::MiddleLeft | EIslandFloorNeighbours::TopLeft:
			rotationOffset.Yaw = 90.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			SideFloorPieceTransforms.Add(newTransform);
			break;
		case EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleLeft:
		case EIslandFloorNeighbours::TopLeft | EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleLeft:
			FullFloorPieceTransforms.Add(newTransform);
			break;
		case EIslandFloorNeighbours::TopLeft:
		default:
			ConvexCornerTransforms.Add(newTransform);
		}
	}
}

void AIslandGenerationTest::SetTopRightFloorShape(int x, int y, int width)
{
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	int index = x + (y * width);
	FVector posOffset = FVector((x * 2) + 1.0f, (y * 2), 0.0f) * (tileScale * 2) + GetActorLocation();
	FRotator rotationOffset = FRotator(0.0f, 0.0f, 0.0f);
	float scaleOffset = tileScale * 2;
	FTransform newTransform;

	newTransform.SetLocation(posOffset);
	newTransform.SetRotation(rotationOffset.Quaternion());
	newTransform.SetScale3D(FVector(scaleOffset));

	EIslandFloorNeighbours topRightCornerCheck = FloorValuesArray[index].FloorNeighbours & (EIslandFloorNeighbours::TopRight | EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleRight);

	if (FloorValuesArray[index].IsFullTile)
	{
		switch (topRightCornerCheck)
		{
		case EIslandFloorNeighbours::TopMiddle:
		case EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::TopRight:
			rotationOffset.Yaw = 180.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			SideFloorPieceTransforms.Add(newTransform);
			break;
		case EIslandFloorNeighbours::MiddleRight:
		case EIslandFloorNeighbours::MiddleRight | EIslandFloorNeighbours::TopRight:
			rotationOffset.Yaw = 90.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			SideFloorPieceTransforms.Add(newTransform);
			break;
		case EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleRight:
		case EIslandFloorNeighbours::TopRight | EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleRight:
			FullFloorPieceTransforms.Add(newTransform);
			break;
		case EIslandFloorNeighbours::TopRight:
		default:
			rotationOffset.Yaw = 90.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			ConvexCornerTransforms.Add(newTransform);
			break;
		}
	}
}

void AIslandGenerationTest::SetBottomLeftFloorShape(int x, int y, int width)
{
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	int index = x + (y * width);
	FVector posOffset = FVector((x * 2), (y * 2) + 1.0f, 0.0f) * (tileScale * 2) + GetActorLocation();
	FRotator rotationOffset = FRotator(0.0f, 0.0f, 0.0f);
	float scaleOffset = tileScale * 2;
	FTransform newTransform;

	newTransform.SetLocation(posOffset);
	newTransform.SetRotation(rotationOffset.Quaternion());
	newTransform.SetScale3D(FVector(scaleOffset));

	EIslandFloorNeighbours bottomLeftCornerCheck = FloorValuesArray[index].FloorNeighbours & (EIslandFloorNeighbours::BottomLeft | EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleLeft);

	if (FloorValuesArray[index].IsFullTile)
	{
		switch (bottomLeftCornerCheck)
		{
		case EIslandFloorNeighbours::BottomMiddle:
		case EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::BottomLeft:
			SideFloorPieceTransforms.Add(newTransform);
			break;
		case EIslandFloorNeighbours::MiddleLeft:
		case EIslandFloorNeighbours::MiddleLeft | EIslandFloorNeighbours::BottomLeft:
			rotationOffset.Yaw = -90.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			SideFloorPieceTransforms.Add(newTransform);
			break;
		case EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleLeft:
		case EIslandFloorNeighbours::BottomLeft | EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleLeft:
			FullFloorPieceTransforms.Add(newTransform);
			break;
		case EIslandFloorNeighbours::BottomLeft:
		default:
			rotationOffset.Yaw = -90.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			ConvexCornerTransforms.Add(newTransform);
			break;
		}
	}
}

void AIslandGenerationTest::SetBottomRightFloorShape(int x, int y, int width)
{
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	int index = x + (y * width);
	FVector posOffset = FVector((x * 2) + 1.0, (y * 2) + 1.0f, 0.0f) * (tileScale * 2) + GetActorLocation();
	FRotator rotationOffset = FRotator(0.0f, 0.0f, 0.0f);
	float scaleOffset = tileScale * 2;
	FTransform newTransform;

	newTransform.SetLocation(posOffset);
	newTransform.SetRotation(rotationOffset.Quaternion());
	newTransform.SetScale3D(FVector(scaleOffset));

	EIslandFloorNeighbours bottomRightCornerCheck = FloorValuesArray[index].FloorNeighbours & (EIslandFloorNeighbours::BottomRight | EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleRight);

	if (FloorValuesArray[index].IsFullTile)
	{
		switch (bottomRightCornerCheck)
		{
		case EIslandFloorNeighbours::BottomMiddle:
		case EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::BottomRight:
			rotationOffset.Yaw = 180.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			SideFloorPieceTransforms.Add(newTransform);
			break;
		case EIslandFloorNeighbours::MiddleRight:
		case EIslandFloorNeighbours::MiddleRight | EIslandFloorNeighbours::BottomRight:
			rotationOffset.Yaw = -90.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			SideFloorPieceTransforms.Add(newTransform);
			break;
		case EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleRight:
		case EIslandFloorNeighbours::BottomRight | EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleRight:
			FullFloorPieceTransforms.Add(newTransform);
			break;
		case EIslandFloorNeighbours::BottomRight:
		default:
			rotationOffset.Yaw = 180.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			ConvexCornerTransforms.Add(newTransform);
			break;
		}
	}
}

void AIslandGenerationTest::SetWall(int tileIndex, FVector posOffset, TSubclassOf<class APackedLevelActor> CornerWall, TSubclassOf<class APackedLevelActor> SideWall)
{
	SetTopLeftWall(tileIndex, posOffset, CornerWall, SideWall);
	SetTopRightWall(tileIndex, posOffset, CornerWall, SideWall);
	SetBottomLeftWall(tileIndex, posOffset, CornerWall, SideWall);
	SetBottomRightWall(tileIndex, posOffset, CornerWall, SideWall);
}

void AIslandGenerationTest::SetTopLeftWall(int tileIndex, FVector posOffset, TSubclassOf<class APackedLevelActor> CornerWall, TSubclassOf<class APackedLevelActor> SideWall)
{
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	FVector newPos = FVector(posOffset.X - tileScale, posOffset.Y - tileScale, 0.0f);

	EIslandFloorNeighbours topLeftCornerCheck = FloorValuesArray[tileIndex].FloorNeighbours & (EIslandFloorNeighbours::TopLeft | EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleLeft);

	switch (topLeftCornerCheck)
	{
	case EIslandFloorNeighbours::TopMiddle:
	case EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::TopLeft:
		if (SideWall)
		{
			APackedLevelActor* side = GetWorld()->SpawnActor<APackedLevelActor>(SideWall, newPos, FRotator(0.0f, 0.0f, 0.0f));
		}
		break;
	case EIslandFloorNeighbours::MiddleLeft:
	case EIslandFloorNeighbours::MiddleLeft | EIslandFloorNeighbours::TopLeft:
		if (SideWall)
		{
			APackedLevelActor* side = GetWorld()->SpawnActor<APackedLevelActor>(SideWall, newPos, FRotator(0.0f, 90.0f, 0.0f));
		}
		break;
	case EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleLeft:
	case EIslandFloorNeighbours::TopLeft | EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleLeft:
		break;
	case EIslandFloorNeighbours::TopLeft:
		if (CornerWall)
		{
			APackedLevelActor* corner = GetWorld()->SpawnActor<APackedLevelActor>(CornerWall, newPos, FRotator(0.0f, 0.0f, 0.0f));
		}
		break;
	default:
		if (CornerWall)
		{
			APackedLevelActor* corner = GetWorld()->SpawnActor<APackedLevelActor>(CornerWall, newPos, FRotator(0.0f, 0.0f, 0.0f));
		}
	}
}

void AIslandGenerationTest::SetTopRightWall(int tileIndex, FVector posOffset, TSubclassOf<class APackedLevelActor> CornerWall, TSubclassOf<class APackedLevelActor> SideWall)
{
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	FVector newPos = FVector(posOffset.X + tileScale, posOffset.Y - tileScale, 0.0f);

	EIslandFloorNeighbours topRightCornerCheck = FloorValuesArray[tileIndex].FloorNeighbours & (EIslandFloorNeighbours::TopRight | EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleRight);

	switch (topRightCornerCheck)
	{
	case EIslandFloorNeighbours::TopMiddle:
	case EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::TopRight:
		if (SideWall)
		{
			APackedLevelActor* side = GetWorld()->SpawnActor<APackedLevelActor>(SideWall, newPos, FRotator(0.0f, 180.0f, 0.0f));
		}
		break;
	case EIslandFloorNeighbours::MiddleRight:
	case EIslandFloorNeighbours::MiddleRight | EIslandFloorNeighbours::TopRight:
		if (SideWall)
		{
			APackedLevelActor* side = GetWorld()->SpawnActor<APackedLevelActor>(SideWall, newPos, FRotator(0.0f, 90.0f, 0.0f));
		}
		break;
	case EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleRight:
	case EIslandFloorNeighbours::TopRight | EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleRight:
		break;
	case EIslandFloorNeighbours::TopRight:
		if (CornerWall)
		{
			APackedLevelActor* corner = GetWorld()->SpawnActor<APackedLevelActor>(CornerWall, newPos, FRotator(0.0f, 90.0f, 0.0f));
		}
		break;
	default:
		if (CornerWall)
		{
			APackedLevelActor* corner = GetWorld()->SpawnActor<APackedLevelActor>(CornerWall, newPos, FRotator(0.0f, 90.0f, 0.0f));
		}
		break;
	}
}

void AIslandGenerationTest::SetBottomLeftWall(int tileIndex, FVector posOffset, TSubclassOf<class APackedLevelActor> CornerWall, TSubclassOf<class APackedLevelActor> SideWall)
{
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	FVector newPos = FVector(posOffset.X - tileScale, posOffset.Y + tileScale, 0.0f);

	EIslandFloorNeighbours bottomLeftCornerCheck = FloorValuesArray[tileIndex].FloorNeighbours & (EIslandFloorNeighbours::BottomLeft | EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleLeft);

	switch (bottomLeftCornerCheck)
	{
	case EIslandFloorNeighbours::BottomMiddle:
	case EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::BottomLeft:
		if (SideWall)
		{
			APackedLevelActor* side = GetWorld()->SpawnActor<APackedLevelActor>(SideWall, newPos, FRotator(0.0f, 0.0f, 0.0f));
		}
		break;
	case EIslandFloorNeighbours::MiddleLeft:
	case EIslandFloorNeighbours::MiddleLeft | EIslandFloorNeighbours::BottomLeft:
		if (SideWall)
		{
			APackedLevelActor* side = GetWorld()->SpawnActor<APackedLevelActor>(SideWall, newPos, FRotator(0.0f, 270.0f, 0.0f));
		}
		break;
	case EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleLeft:
	case EIslandFloorNeighbours::BottomLeft | EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleLeft:
		break;
	case EIslandFloorNeighbours::BottomLeft:
		if (CornerWall)
		{
			APackedLevelActor* corner = GetWorld()->SpawnActor<APackedLevelActor>(CornerWall, newPos, FRotator(0.0f, 270.0f, 0.0f));
		}
		break;
	default:
		if (CornerWall)
		{
			APackedLevelActor* corner = GetWorld()->SpawnActor<APackedLevelActor>(CornerWall, newPos, FRotator(0.0f, 270.0f, 0.0f));
		}
		break;
	}
}

void AIslandGenerationTest::SetBottomRightWall(int tileIndex, FVector posOffset, TSubclassOf<class APackedLevelActor> CornerWall, TSubclassOf<class APackedLevelActor> SideWall)
{
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	FVector newPos = FVector(posOffset.X + tileScale, posOffset.Y + tileScale, 0.0f);

	EIslandFloorNeighbours bottomRightCornerCheck = FloorValuesArray[tileIndex].FloorNeighbours & (EIslandFloorNeighbours::BottomRight | EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleRight);

	switch (bottomRightCornerCheck)
	{
	case EIslandFloorNeighbours::BottomMiddle:
	case EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::BottomRight:
		if (SideWall)
		{
			APackedLevelActor* side = GetWorld()->SpawnActor<APackedLevelActor>(SideWall, newPos, FRotator(0.0f, 180.0f, 0.0f));
		}
		break;
	case EIslandFloorNeighbours::MiddleRight:
	case EIslandFloorNeighbours::MiddleRight | EIslandFloorNeighbours::BottomRight:
		if (SideWall)
		{
			APackedLevelActor* side = GetWorld()->SpawnActor<APackedLevelActor>(SideWall, newPos, FRotator(0.0f, -90.0f, 0.0f));
		}
		break;
	case EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleRight:
	case EIslandFloorNeighbours::BottomRight | EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleRight:
		break;
	case EIslandFloorNeighbours::BottomRight:
		if (CornerWall)
		{
			APackedLevelActor* corner = GetWorld()->SpawnActor<APackedLevelActor>(CornerWall, newPos, FRotator(0.0f, 180.0f, 0.0f));
		}
		break;
	default:
		if (CornerWall)
		{
			APackedLevelActor* corner = GetWorld()->SpawnActor<APackedLevelActor>(CornerWall, newPos, FRotator(0.0f, 180.0f, 0.0f));
		}
		break;
	}
}

void AIslandGenerationTest::Server_SpawnFloorObstacles_Implementation(int x, int y, int width)
{
	SpawnFloorObstacles(x, y, width);
}

void AIslandGenerationTest::SpawnFloorObstacles(int x, int y, int width)
{
	int index = x + (y * width);

	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	float yaw = FloorValuesArray[index].obstacleYaw;
	FVector posOffset = FVector((tileScale * x * 4) + tileScale, (tileScale * y * 4) + tileScale, 0.0f) + GetActorLocation();

	TArray<TSubclassOf<APackedLevelActor>> actorArray = GenerationValuesComponent->ObstacleValues.ObstacleActor;
	TArray<TSubclassOf<APackedLevelActor>> vantageArray = GenerationValuesComponent->VantageValues.VantageActor;

	TSubclassOf<class APackedLevelActor> obstacleWallTemplate = GenerationValuesComponent->ObstacleValues.ObstacleWall[FloorValuesArray[index].randomFloorToSpawn];
	TSubclassOf<class APackedLevelActor> basicWallTemplate = GenerationValuesComponent->IslandValues.BasicWall;
	TSubclassOf<class APackedLevelActor> cornerWallTemplate = GenerationValuesComponent->IslandValues.CornerWall;
	TSubclassOf<class APackedLevelActor> startEndWallTemplate = GenerationValuesComponent->IslandStartEnd.StartEndWall;

	switch (FloorValuesArray[index].IslandFloorObstacle)
	{
	case EIslandFloorObstacle::Basic:
		if (actorArray.IsValidIndex(FloorValuesArray[index].randomFloorToSpawn))
		{
			APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(actorArray[FloorValuesArray[index].randomFloorToSpawn], posOffset, FRotator(0.0f, yaw, 0.0f));
		}

		if (obstacleWallTemplate != nullptr)
		{
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::MiddleLeft))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(obstacleWallTemplate, posOffset, FRotator(0.0f, 0.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::TopMiddle))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(obstacleWallTemplate, posOffset, FRotator(0.0f, 90.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::MiddleRight))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(obstacleWallTemplate, posOffset, FRotator(0.0f, 180.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::BottomMiddle))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(obstacleWallTemplate, posOffset, FRotator(0.0f, 270.0f, 0.0f));
			}
		}

		break;
		//case EFloorObstacle::Hole:
		//	if (HoleTilePackedActorArray.IsValidIndex(FloorValuesArray[index].randomFloorToSpawn))
		//	{
		//		APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(HoleTilePackedActorArray[FloorValuesArray[index].randomFloorToSpawn], posOffset, FRotator(0.0f, yaw, 0.0f));
		//	}

		//	if (HoleTilePackedActorWallArray.IsValidIndex(FloorValuesArray[index].randomFloorToSpawn))
		//	{
		//		if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
		//		{
		//			APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(HoleTilePackedActorWallArray[FloorValuesArray[index].randomFloorToSpawn], posOffset, FRotator(0.0f, 0.0f, 0.0f));
		//		}
		//		if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
		//		{
		//			APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(HoleTilePackedActorWallArray[FloorValuesArray[index].randomFloorToSpawn], posOffset, FRotator(0.0f, 90.0f, 0.0f));
		//		}
		//		if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
		//		{
		//			APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(HoleTilePackedActorWallArray[FloorValuesArray[index].randomFloorToSpawn], posOffset, FRotator(0.0f, 180.0f, 0.0f));
		//		}
		//		if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
		//		{
		//			APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(HoleTilePackedActorWallArray[FloorValuesArray[index].randomFloorToSpawn], posOffset, FRotator(0.0f, 270.0f, 0.0f));
		//		}
		//	}
		//	break;
	case EIslandFloorObstacle::Vantage:
		if (vantageArray.IsValidIndex(FloorValuesArray[index].randomFloorToSpawn))
		{
			APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(vantageArray[FloorValuesArray[index].randomFloorToSpawn], posOffset, FRotator(0.0f, yaw, 0.0f));
		}

		if (cornerWallTemplate != nullptr && basicWallTemplate != nullptr)
		{
			SetWall(index, posOffset, cornerWallTemplate, basicWallTemplate);
		}

		break;

	case EIslandFloorObstacle::CapturableFlag:
		if (GenerationValuesComponent->FlagObjectiveValues.Flag != nullptr)
		{
			ANewCapturableFlag* newFlag = GetWorld()->SpawnActor<ANewCapturableFlag>(GenerationValuesComponent->FlagObjectiveValues.Flag, posOffset, FRotator(0.0f, yaw, 0.0f));

			newFlag->OnObjectiveCreated.AddDynamic(this, &AIslandGenerationTest::IncrementObjective);
			newFlag->OnObjectiveActivated.AddDynamic(this, &AIslandGenerationTest::DecrementObjective);

			newFlag->Activate();
		}

		if (cornerWallTemplate != nullptr && basicWallTemplate != nullptr)
		{
			SetWall(index, posOffset, cornerWallTemplate, basicWallTemplate);
		}
		break;
		case EIslandFloorObstacle::EnemyZone:
			if (GenerationValuesComponent->EnemyZoneObjectiveValues.EnemyZone != nullptr)
			{
				ANewEnemyZone* newEnemyZone = GetWorld()->SpawnActor<ANewEnemyZone>(GenerationValuesComponent->EnemyZoneObjectiveValues.EnemyZone, posOffset, FRotator(0.0f, yaw, 0.0f));

				newEnemyZone->OnObjectiveCreated.AddDynamic(this, &AIslandGenerationTest::IncrementObjective);
				newEnemyZone->OnObjectiveActivated.AddDynamic(this, &AIslandGenerationTest::DecrementObjective);

				newEnemyZone->Activate();
			}

		if (cornerWallTemplate != nullptr && basicWallTemplate != nullptr)
		{
			SetWall(index, posOffset, cornerWallTemplate, basicWallTemplate);
		}
		break;
	case EIslandFloorObstacle::SurvivalZone:
		if (GenerationValuesComponent->SurvivalZoneObjectiveValues.SurvivalZone != nullptr)
		{
			ANewSurvivalZone* newZone = GetWorld()->SpawnActor<ANewSurvivalZone>(GenerationValuesComponent->SurvivalZoneObjectiveValues.SurvivalZone, posOffset, FRotator(0.0f, yaw, 0.0f));

			newZone->OnObjectiveCreated.AddDynamic(this, &AIslandGenerationTest::IncrementObjective);
			newZone->OnObjectiveActivated.AddDynamic(this, &AIslandGenerationTest::DecrementObjective);

			newZone->Duration = GenerationValuesComponent->SurvivalZoneObjectiveValues.SurvivalDuration;

			newZone->Activate();
		}

		if (cornerWallTemplate != nullptr && basicWallTemplate != nullptr)
		{
			SetWall(index, posOffset, cornerWallTemplate, basicWallTemplate);
		}

		break;
	case EIslandFloorObstacle::Start:
		if (GenerationValuesComponent->IslandStartEnd.LevelStartPackedActor != nullptr && IslandIndex == 0)
		{
			APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(GenerationValuesComponent->IslandStartEnd.LevelStartPackedActor, posOffset, FRotator(0.0f, 0.0f, 0.0f));
		}

		if (startEndWallTemplate != nullptr)
		{
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::MiddleLeft) && IslandIndex == 0)
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(startEndWallTemplate, posOffset, FRotator(0.0f, 0.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::TopMiddle))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(startEndWallTemplate, posOffset, FRotator(0.0f, 90.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::MiddleRight))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(startEndWallTemplate, posOffset, FRotator(0.0f, 180.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::BottomMiddle))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(startEndWallTemplate, posOffset, FRotator(0.0f, 270.0f, 0.0f));
			}
		}

		break;
	case EIslandFloorObstacle::End:
		if (GenerationValuesComponent->IslandStartEnd.LevelEndPackedActor != nullptr)
		{
			APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(GenerationValuesComponent->IslandStartEnd.LevelEndPackedActor, posOffset, FRotator(0.0f, 0.0f, 0.0f));
		}

		if (startEndWallTemplate != nullptr)
		{
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::MiddleLeft))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(startEndWallTemplate, posOffset, FRotator(0.0f, 0.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::TopMiddle))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(startEndWallTemplate, posOffset, FRotator(0.0f, 90.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::MiddleRight))
			{
				//APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(startEndWallTemplate, posOffset, FRotator(0.0f, 180.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::BottomMiddle))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(startEndWallTemplate, posOffset, FRotator(0.0f, 270.0f, 0.0f));
			}
		}

		break;
	case EIslandFloorObstacle::None:

		if (cornerWallTemplate != nullptr && basicWallTemplate != nullptr)
		{
			if (FloorValuesArray[index].IsFullTile)
			{
				SetWall(index, posOffset, cornerWallTemplate, basicWallTemplate);
			}
		}

		break;
	}
}

void AIslandGenerationTest::CreateFloor()
{
	TArray<TArray<FTransform>> arrayOfTransformArrays;
	arrayOfTransformArrays.Add(FullFloorPieceTransforms);
	arrayOfTransformArrays.Add(SideFloorPieceTransforms);
	arrayOfTransformArrays.Add(ConvexCornerTransforms);

	TArray<UStaticMesh*> staticMeshArray;
	staticMeshArray.Add(GenerationValuesComponent->FloorMeshes.FullFloorMesh);
	staticMeshArray.Add(GenerationValuesComponent->FloorMeshes.SideFloorMesh);
	staticMeshArray.Add(GenerationValuesComponent->FloorMeshes.CornerFloorMesh);

	for (int i = 0; i < staticMeshArray.Num(); i++)
	{
		UInstancedStaticMeshComponent* ISMComp = NewObject<UInstancedStaticMeshComponent>(this);

		if (staticMeshArray[i] != nullptr)
		{
			ISMComp->SetStaticMesh(staticMeshArray[i]);
		}
		if (GenerationValuesComponent->FloorMeshes.FloorMaterial != nullptr)
		{
			ISMComp->SetMaterial(0, GenerationValuesComponent->FloorMeshes.FloorMaterial);
		}

		ISMComp->AddInstances(arrayOfTransformArrays[i], false, true, true);

		ISMComp->RegisterComponent();
	}
}

void AIslandGenerationTest::IncrementObjective()
{
	CurrentObjectives++;
	FString objectiveName = "Nothing";

	switch (IslandObjective)
	{
	case EIslandObjectiveType::CaptureTheFlag:
		objectiveName = "Flags";
		break;
	case EIslandObjectiveType::DefeatAllEnemies:
		objectiveName = "Enemies";
		break;
	case EIslandObjectiveType::MoveThePayload:
		objectiveName = "Payload";
		break;
	case EIslandObjectiveType::Survival:
		objectiveName = "Survival Zones";
		break;
	}

	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, FString::Printf(TEXT("Island: %i | Remaining objectives: % i | "), IslandIndex, CurrentObjectives) + objectiveName);
}

void AIslandGenerationTest::DecrementObjective()
{
	CurrentObjectives--;
	FString objectiveName = "Nothing";

	switch (IslandObjective)
	{
	case EIslandObjectiveType::CaptureTheFlag:
		objectiveName = "Flags";
		break;
	case EIslandObjectiveType::DefeatAllEnemies:
		objectiveName = "Enemies";
		break;
	case EIslandObjectiveType::MoveThePayload:
		objectiveName = "Payload";
		break;
	case EIslandObjectiveType::Survival:
		objectiveName = "Survival Zones";
		break;
	}

	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, FString::Printf(TEXT("Island: %i | Remaining objectives: % i | "), IslandIndex, CurrentObjectives) + objectiveName);
	if (CurrentObjectives <= 0)
	{
		if (Bridge)
		{
			Bridge->Activate();
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, FString::Printf(TEXT("Bridge Activated")));
		}

		OnObjectiveIslandObjectiveComplete.Broadcast();
	}
}

void AIslandGenerationTest::OnRep_FloorValuesArrayChange()
{
	for (int y = 0; y < IslandDepth; y++)
	{
		for (int x = 0; x < IslandWidth; x++)
		{
			CheckFloor(x, y, IslandWidth, IslandDepth);
		}
	}
	CreateFloor();
}

void AIslandGenerationTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AIslandGenerationTest, FloorValuesArray);
	DOREPLIFETIME(AIslandGenerationTest, IslandWidth);
	DOREPLIFETIME(AIslandGenerationTest, IslandDepth);
	DOREPLIFETIME(AIslandGenerationTest, GenerationValuesComponent);
}