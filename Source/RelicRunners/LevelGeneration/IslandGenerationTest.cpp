// Fill out your copyright notice in the Description page of Project Settings.


#include "IslandGenerationTest.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "IslandSpawningValuesComponent.h"
#include "Net/UnrealNetwork.h"
#include "PackedLevelActor/PackedLevelActor.h"
#include "CapturableFlag.h"
#include "EnemyZone.h"
#include "PayloadActor.h"
#include "SurvivalZone.h"

/*
Set index of starting tile and ending (bridge) tile

Initialize values of each floor tile based on width and depth + set what regular obstacle will spawn based on a precentage

Override previously defined tiles with starting and ending tiles

Based on maximum amount, override previously defined tiles (excluding the starting and ending tiles) with the objective of that island
Spawn the objective at the appropriate location

Spawn regular obstacles at their appropriate location + Spawn their appropriate walls with instant static mesh

*/


// Sets default values
AIslandGenerationTest::AIslandGenerationTest()
{
	PrimaryActorTick.bCanEverTick = false;

	GenerationValuesComponent = CreateDefaultSubobject<UIslandSpawningValuesComponent>("GenerationValues");

	bReplicates = true;
	bAlwaysRelevant = true;
}

void AIslandGenerationTest::Generate()
{
	if (HasAuthority())
	{
		//if (IslandOwner)
		//{
		//	int num = IslandOwner->GetObjectiveValue();
		//	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Current Objective: %i"), num));
		//}
			
		//Initialize the starting and ending tiles.
		int startingIndex = 0, endingIndex = 0;

		//If somehow both are at the exact same index, reinitialize starting and ending tiles
		do
		{
			startingIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);
			endingIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

		} while (CalculateDistanceBetweenTiles(startingIndex, endingIndex) || startingIndex == endingIndex);

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

			FindStartToEndPath(startingIndex, endingIndex, IslandWidth, true);
		}

		InitializeObjectives(startingIndex, endingIndex);

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
	}
}

void AIslandGenerationTest::InitializeValues(EIslandObjectiveType islandObjective)
{
	IslandDepth = FMath::RandRange(GenerationValuesComponent->IslandValues.DepthMin, GenerationValuesComponent->IslandValues.DepthMax);
	IslandWidth = FMath::RandRange(GenerationValuesComponent->IslandValues.WidthMin, GenerationValuesComponent->IslandValues.WidthMax);

	IslandObjective = islandObjective;
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

void AIslandGenerationTest::ForceFloorBool(bool forcedFloor, int x, int y, int width)
{
	int index = x + (y * width);

	if (!EnumHasAnyFlags(FloorValuesArray[index].IslandFloorObstacle, EIslandFloorObstacle::Hole))
	{
		FloorValuesArray[index].IsFullTile = forcedFloor;

		if (forcedFloor)
		{
			if (FloorValuesArray[index].IslandFloorObstacle == EIslandFloorObstacle::None)
			{
				InitializeModularObstacle(FloorValuesArray[index], false);
			}
		}
	}
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
			ForceFloorBool(true, currentX, currentY, width);
		}
	}
}

void AIslandGenerationTest::FindStartToEndPath(int startingIndex, int targetIndex, int width, bool implementSpline)
{
}

void AIslandGenerationTest::InitializeObjectives(int startingIndex, int endingIndex)
{
	switch (IslandObjective)
	{
	case EIslandObjectiveType::CaptureTheFlag:
		int flagAmount = FMath::RandRange(GenerationValuesComponent->FlagObjectiveValues.FlagAmountMin, GenerationValuesComponent->FlagObjectiveValues.FlagAmountMax);

		//Override previously initialized tiles with capturable flag tiles
		for (int i = 0; i < flagAmount; i++)
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
		int enemyZoneAmount = FMath::RandRange(GenerationValuesComponent->EnemyZoneObjectiveValues.FlagAmountMin, GenerationValuesComponent->EnemyZoneObjectiveValues.FlagAmountMax);

		for (int i = 0; i < enemyZoneAmount; i++)
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
		if (IslandDepth * IslandWidth > 1)
		{
			//Override starting and ending tiles initialized above and create a path from one to the other.
			SetStartingAndEndingPoints(startingIndex, endingIndex);

			FindStartToEndPath(startingIndex, endingIndex, IslandWidth, true);

			if (GenerationValuesComponent->PayloadObjectiveValues.Payload != nullptr)
			{
				APayloadActor* payload = GetWorld()->SpawnActor<APayloadActor>(GenerationValuesComponent->PayloadObjectiveValues.Payload, FVector::ZeroVector, FRotator::ZeroRotator);
				//payload->Multicast_Initialize(DeliverySpline, GenerationValuesComponent->PayloadObjectiveValues.PayloadSpeed);
			}
		}
		break;
	case EIslandObjectiveType::Survival:
		int survivalZoneAmount = FMath::RandRange(GenerationValuesComponent->SurvivalZoneObjectiveValues.ZoneAmountMin, GenerationValuesComponent->SurvivalZoneObjectiveValues.ZoneAmountMax);

		//Override previously initialized tiles with survival zone tiles
		for (int i = 0; i < survivalZoneAmount; i++)
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

	TArray<TSubclassOf<APackedLevelActor>> actorArray = GenerationValuesComponent->ObstacleValues.ObstacleActor.Array();
	TArray<TSubclassOf<APackedLevelActor>> vantageArray = GenerationValuesComponent->VantageValues.VantageActor.Array();

	switch (FloorValuesArray[index].IslandFloorObstacle)
	{
	case EIslandFloorObstacle::Basic:
		if (actorArray.IsValidIndex(FloorValuesArray[index].randomFloorToSpawn))
		{
			APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(actorArray[FloorValuesArray[index].randomFloorToSpawn], posOffset, FRotator(0.0f, yaw, 0.0f));
		}

		//if (PackedActorBasicWall != nullptr)
		//{
		//	if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
		//	{
		//		APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 0.0f, 0.0f));
		//	}
		//	if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
		//	{
		//		APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 90.0f, 0.0f));
		//	}
		//	if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
		//	{
		//		APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 180.0f, 0.0f));
		//	}
		//	if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
		//	{
		//		APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 270.0f, 0.0f));
		//	}
		//}
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

		//if (!VantagePackedActorArray.IsEmpty())
		//{
		//	int randIndex = FMath::RandRange(0, VantagePackedActorArray.Num() - 1);

		//	if (VantagePackedActorArray.IsValidIndex(randIndex))
		//	{
		//		if (VantagePackedActorArray[randIndex])
		//		{
		//			APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(VantagePackedActorArray[randIndex], posOffset, FRotator(0.0f, yaw, 0.0f));
		//		}
		//	}

	//		if (PackedActorBasicWall != nullptr)
	//		{
	//			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
	//			{
	//				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 0.0f, 0.0f));
	//			}
	//			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
	//			{
	//				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 90.0f, 0.0f));
	//			}
	//			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
	//			{
	//				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 180.0f, 0.0f));
	//			}
	//			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
	//			{
	//				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 270.0f, 0.0f));
	//			}
	//		}
	//	}
		break;

	case EIslandFloorObstacle::CapturableFlag:
		if (GenerationValuesComponent->FlagObjectiveValues.Flag != nullptr)
		{
			ACapturableFlag* newFlag = GetWorld()->SpawnActor<ACapturableFlag>(GenerationValuesComponent->FlagObjectiveValues.Flag, posOffset, FRotator(0.0f, yaw, 0.0f));
			
			newFlag->OnFlagObjectiveCreated.AddDynamic(this, &AIslandGenerationTest::IncrementObjective);
			newFlag->OnFlagObjectiveActivated.AddDynamic(this, &AIslandGenerationTest::DecrementObjective);

			newFlag->Activate();
		}

	//	if (EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::CaptureTheFlag))
	//	{
	//		if (!CapturableFlagPackedActorArray.IsEmpty())
	//		{
	//			int randIndex = FMath::RandRange(0, CapturableFlagPackedActorArray.Num() - 1);

	//			if (CapturableFlagPackedActorArray.IsValidIndex(randIndex))
	//			{
	//				APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(CapturableFlagPackedActorArray[randIndex], posOffset, FRotator(0.0f, yaw, 0.0f));
	//			}
	//		}

	//		if (PackedActorBasicWall != nullptr)
	//		{
	//			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
	//			{
	//				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 0.0f, 0.0f));
	//			}
	//			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
	//			{
	//				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 90.0f, 0.0f));
	//			}
	//			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
	//			{
	//				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 180.0f, 0.0f));
	//			}
	//			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
	//			{
	//				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 270.0f, 0.0f));
	//			}
	//		}

	//	}
		break;
	//case EFloorObstacle::EnemyZone:
	//	if (EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::DefeatAllEnemies))
	//	{
	//		if (!EnemyZonePackedActorArray.IsEmpty())
	//		{
	//			int randIndex = FMath::RandRange(0, EnemyZonePackedActorArray.Num() - 1);

	//			if (EnemyZonePackedActorArray.IsValidIndex(randIndex))
	//			{
	//				APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(EnemyZonePackedActorArray[randIndex], posOffset, FRotator(0.0f, yaw, 0.0f));
	//			}
	//		}

	//		if (PackedActorBasicWall != nullptr)
	//		{
	//			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
	//			{
	//				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 0.0f, 0.0f));
	//			}
	//			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
	//			{
	//				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 90.0f, 0.0f));
	//			}
	//			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
	//			{
	//				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 180.0f, 0.0f));
	//			}
	//			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
	//			{
	//				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 270.0f, 0.0f));
	//			}
	//		}

	//	}
	//	break;
	case EIslandFloorObstacle::SurvivalZone:
	 		if (GenerationValuesComponent->FlagObjectiveValues.Flag != nullptr)
		{
			ASurvivalZone* newZone = GetWorld()->SpawnActor<ASurvivalZone>(GenerationValuesComponent->SurvivalZoneObjectiveValues.SurvivalZone, posOffset, FRotator(0.0f, yaw, 0.0f));
			
			newZone->OnFlagObjectiveCreated.AddDynamic(this, &AIslandGenerationTest::IncrementObjective);
			newZone->OnFlagObjectiveActivated.AddDynamic(this, &AIslandGenerationTest::DecrementObjective);

			newZone->Activate();
		}

	//	if (EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::Survival))
	//	{
	//		if (!SurvivalZonePackedActorArray.IsEmpty())
	//		{
	//			int randIndex = FMath::RandRange(0, SurvivalZonePackedActorArray.Num() - 1);

	//			if (SurvivalZonePackedActorArray.IsValidIndex(randIndex))
	//			{
	//				APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(SurvivalZonePackedActorArray[randIndex], posOffset, FRotator(0.0f, yaw, 0.0f));
	//			}
	//		}

	//		if (PackedActorBasicWall != nullptr)
	//		{
	//			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
	//			{
	//				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 0.0f, 0.0f));
	//			}
	//			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
	//			{
	//				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 90.0f, 0.0f));
	//			}
	//			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
	//			{
	//				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 180.0f, 0.0f));
	//			}
	//			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
	//			{
	//				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 270.0f, 0.0f));
	//			}
	//		}

	//	}
		break;
	case EIslandFloorObstacle::Start:
		if (GenerationValuesComponent->IslandStartEnd.LevelStartPackedActor != nullptr && IslandIndex == 0)
		{
			APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(GenerationValuesComponent->IslandStartEnd.LevelStartPackedActor, posOffset, FRotator(0.0f, yaw, 0.0f));
		}

		//if (EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::MoveThePayload))
		//{
		//	DeliverySpline->SetLocationAtSplinePoint(0, posOffset, ESplineCoordinateSpace::World);
		//}

		//if (PackedActorBasicWall != nullptr)
		//{
		//	if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
		//	{
		//		APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 0.0f, 0.0f));
		//	}
		//	if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
		//	{
		//		APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 90.0f, 0.0f));
		//	}
		//	if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
		//	{
		//		APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 180.0f, 0.0f));
		//	}
		//	if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
		//	{
		//		APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 270.0f, 0.0f));
		//	}
		//}

		break;
	case EIslandFloorObstacle::End:
		if (GenerationValuesComponent->IslandStartEnd.LevelEndPackedActor != nullptr)
		{
			APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(GenerationValuesComponent->IslandStartEnd.LevelEndPackedActor, posOffset, FRotator(0.0f, yaw, 0.0f));
		}

		//if (LevelChangeTrigger != nullptr)
		//{
		//	ALevelChangeTrigger* levelChangeActor = GetWorld()->SpawnActor<ALevelChangeTrigger>(LevelChangeTrigger, posOffset, FRotator(0.0f, yaw, 0.0f));

		//	if (!TargetLevel.IsNull())
		//	{
		//		levelChangeActor->Server_SetTargetLevel(FName(TargetLevel.GetAssetName()));
		//	}
		//}

		//if (PackedActorBasicWall != nullptr)
		//{
		//	if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
		//	{
		//		APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 0.0f, 0.0f));
		//	}
		//	if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
		//	{
		//		APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 90.0f, 0.0f));
		//	}
		//	if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
		//	{
		//		APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 180.0f, 0.0f));
		//	}
		//	if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
		//	{
		//		APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 270.0f, 0.0f));
		//	}
		//}

	//	break;
	//case EFloorObstacle::None:

	//	if (PackedActorBasicWall != nullptr && FloorValuesArray[index].IsFullTile)
	//	{
	//		if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
	//		{
	//			APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 0.0f, 0.0f));
	//		}
	//		if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
	//		{
	//			APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 90.0f, 0.0f));
	//		}
	//		if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
	//		{
	//			APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 180.0f, 0.0f));
	//		}
	//		if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
	//		{
	//			APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 270.0f, 0.0f));
	//		}
	//	}
	//	break;
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
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, FString::Printf(TEXT("Island: %i | Remaining flags: %i"), IslandIndex, CurrentObjectives));
}

void AIslandGenerationTest::DecrementObjective()
{
	CurrentObjectives--;
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, FString::Printf(TEXT("Island: %i | Remaining flags: %i"), IslandIndex, CurrentObjectives));
	if (CurrentObjectives <= 0)
	{
		//BridgeActor->Activate();
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, FString::Printf(TEXT("Bridge Activated")));
	}
}

void AIslandGenerationTest::OnRep_FloorValuesArrayChange()
{
	//for (int y = 0; y < DepthRange.Final; y++)
	//{
	//	for (int x = 0; x < DepthRange.Final; x++)
	//	{
	//		CheckFloor(x, y, WidthRange.Final, DepthRange.Final);
	//	}
	//}
	//CreateFloor();
}

void AIslandGenerationTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AIslandGenerationTest, FloorValuesArray);
}