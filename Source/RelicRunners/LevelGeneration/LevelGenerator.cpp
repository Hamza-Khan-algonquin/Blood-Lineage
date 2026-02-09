// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelGenerator.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Net/UnrealNetwork.h"
#include "PackedLevelActor/PackedLevelActor.h"
#include "LevelChangeTrigger.h"
#include "PayloadActor.h"
#include "NavigationSystem.h"
#include "RelicRunners/RelicRunnersGameState.h"
/*
TODO:
Fix wall spawning -> Have specific walls for tiles without a pre-determined wall | Done

Make it so that level generation is done within ranges | Done

Adjust how the level generation with the textures work

Make it so that the level generation can generate multiple islands that are connected to each other by bridge | Done
Each island has their own objective | Done
Bridges block the way until the objective is complete | Done

Maybe Make new zones spawn dynamically when the bridge falls | Done

Add map border that becomes more visible the closer you get to it to visualize where the edge of the island is
Add beacon to make it easier to see the locations of objectives from far away. The closer you get, the less visible it becomes

Remove concave corners | Done
Make convex corners square instead of round | Done
Adjust what type of floor spawns to accomadate not having concave corners | Done
Change how the floor tiles are scaled to make them more dynamic | Done

Create Objective parent class where each objective derives from | Done

*/


/*
Plan for level generation

Island Spawner Spawns the islands
Initialize each islands' position + values
Each individual island then begins to generate with the values


Values to send each island:

Basic Values:
	Map width + Map Depth
	Tile Scale

Visual Values:
	Floor Meshes (Floor, Wall, Corner)(With wall, No wall)
	Appropriate Floor Material

Obstacle Values:
	Array of packed level obstacles
	Array of packed level bridges
	Packed Level Start
	Bridge actor used to travel between islands


Objective Values:
	Objective Type

	Objective Actor to Spawn
	Max Objective Amount


Objective Actor Parent Class
	Child classes:
	 Capture the flag,
	 Enemy Zone,
	 Deliver the payload,
	 Survive the Zone


*/


ALevelGenerator::ALevelGenerator() :
	TileScale(1.0f),
	CameraActor(nullptr),
	LevelChangeTrigger(nullptr),
	TargetLevel(nullptr),
	GenerationIsRandom(true),
	LevelTexture(nullptr),
	StartingToEndingMinDistance(0),
	FullPercentage(75.0f),
	HolePercentage(0.0f),
	BasicObstaclePercentage(50.0f),
	CenterForceFull(0),
	BorderForceFull(0),
	LevelStartPackedActor(nullptr),
	LevelEndPackedActor(nullptr),
	FullFloorMesh(nullptr),
	SideFloorMesh(nullptr),
	ConvexCornerMesh(nullptr),
	FloorMaterial(nullptr),
	ObjectiveType(0),
	MaxCapturableFlagAmount(0),
	MaxEnemyZoneAmount(0),
	PayloadActor(nullptr),
	PayloadMovementSpeed(1.0f)
{
	PrimaryActorTick.bCanEverTick = false;

	Origin = CreateDefaultSubobject<USceneComponent>("Origin");
	RootComponent = Origin;

	DeliverySpline = CreateDefaultSubobject<USplineComponent>("Delivery Path");
	DeliverySpline->SetupAttachment(Origin);

	bReplicates = true;
	bAlwaysRelevant = true;
}

void ALevelGenerator::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		//If spawning a level with a texture.
		if (LevelTexture && LevelTexture->GetPlatformData() && !GenerationIsRandom)
		{
			if (LevelTexture->CompressionSettings != TC_VectorDisplacementmap)
			{
				GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, FString::Printf(TEXT
				("Warning!! Texture uses incompatible Compression Setting. Go into the texture's setting, find the *Compression Settings* option and set it to *VectorDisplacementMap.*")));
				return;
			}
			//Find mipmap data from the texture.
			FTexture2DMipMap& MipMap = LevelTexture->GetPlatformData()->Mips[0];
			void* Data = MipMap.BulkData.Lock(LOCK_READ_ONLY);

			//Use the mipmap data to find the width and depth of the texture.
			FColor* FormattedImageData = static_cast<FColor*>(Data);
			int32 textureWidth = MipMap.SizeX;
			int32 textureDepth = MipMap.SizeY;

			//Set Camera Position relative to size of map
			if (CameraActor)
			{
				float cameraX = textureWidth * TileScale * 2 - TileScale;
				float cameraY = textureDepth * TileScale * 2 - TileScale;

				CameraActor->SetActorLocation(FVector(cameraX, cameraY, 1000.0f));
			}

			//Initialize the floor based on each pixel in the texture.
			for (int y = 0; y < textureDepth; y++)
			{
				for (int x = 0; x < textureWidth; x++)
				{
					//Get the colour of the currently checked pixel of the texture.
					FColor PixelColor = FormattedImageData[y * textureWidth + x];
					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Value: %i, HEX: "), y * textureWidth + x) + PixelColor.ToHex());

					FSFloorValues newFloorValues;
					//If current index has a white pixel, set it to unwalkable. Any other colour sets it to walkable.
					newFloorValues.IsFullTile = (PixelColor.ToHex() != FString("ffffffff")); //"ffffffff" -> Hex code for the colour white

					//Add the currently created Floor tile to the array.
					FloorValuesArray.Add(newFloorValues);
				}
			}

			if (!FloorValuesArray.IsEmpty())
			{
				//Initialize the shape of the floor tile and the modular obstacles based on each specified colour
				for (int y = 0; y < textureDepth; y++)
				{
					for (int x = 0; x < textureWidth; x++)
					{
						//Initialize the floor's shape based on the variable set above.
						CheckFloor(x, y, textureWidth, textureDepth);

						//Spawn the modular obstacles based on each specified colour
						FColor PixelColor = FormattedImageData[y * textureWidth + x];
						Server_SpawnFloorObstaclesByColour(x, y, textureWidth, PixelColor);
					}
				}
				//Spawn the floor with the instanced static mesh component.
				CreateFloor();

			}
			MipMap.BulkData.Unlock();
		}
		else
		{
			WidthRange.Final = FMath::RandRange(WidthRange.Min, WidthRange.Max);
			DepthRange.Final = FMath::RandRange(DepthRange.Min, DepthRange.Max);

			//Set Camera Position relative to size of map
			if (CameraActor)
			{
				float cameraX = WidthRange.Final * TileScale * 2 - TileScale;
				float cameraY = DepthRange.Final * TileScale * 2 - TileScale;
				CameraActor->SetActorLocation(FVector(cameraX, cameraY, 0.0f));
			}

			//Initialize the starting and ending tiles.
			int startingIndex = 0, endingIndex = 0;

			//If somehow both are at the exact same index, reinitialize starting and ending tiles
			do
			{
				startingIndex = FMath::RandRange(0, (DepthRange.Final * WidthRange.Final) - 1);
				endingIndex = FMath::RandRange(0, (DepthRange.Final * WidthRange.Final) - 1);

			} while (CalculateDistanceBetweenTiles(startingIndex, endingIndex) || startingIndex == endingIndex);

			for (int y = 0; y < DepthRange.Final; y++)
			{
				for (int x = 0; x < WidthRange.Final; x++)
				{
					//Initialize each tile and set them to either walkable or non-walkable.
					GenerateFloor(x, y);
				}
			}

			if (DepthRange.Final * WidthRange.Final > 1)
			{
				//Override starting and ending tiles initialized above and create a path from one to the other.
				SetStartingAndEndingPoints(startingIndex, endingIndex);

				FindStartToEndPath(startingIndex, endingIndex, WidthRange.Final, true);

				if (EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::MoveThePayload) && PayloadActor != nullptr)
				{
					APayloadActor* payload = GetWorld()->SpawnActor<APayloadActor>(PayloadActor, FVector::ZeroVector, FRotator::ZeroRotator);
					payload->Multicast_Initialize(DeliverySpline, PayloadMovementSpeed);
				}
			}

			//Override previously initialized tiles with capturable flag tiles
			for (int i = 0; i < MaxCapturableFlagAmount; i++)
			{
				int keyTileIndex = FMath::RandRange(0, (DepthRange.Final * WidthRange.Final) - 1);

				int whileLoopLimit = 0;

				//If the currently selected tile can be overriden, do that, if not, find another tile. Repeat 10 time. if failed every time, give up.
				while (FloorValuesArray[keyTileIndex].FloorObstacle > EFloorObstacle::Basic)
				{
					keyTileIndex = FMath::RandRange(0, (DepthRange.Final * WidthRange.Final) - 1);

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

					FindStartToEndPath(startingIndex, keyTileIndex, WidthRange.Final);
				}
			}

			//Override previously initialized tiles with enemy zone tiles
			for (int i = 0; i < MaxEnemyZoneAmount; i++)
			{
				int keyTileIndex = FMath::RandRange(0, (DepthRange.Final * WidthRange.Final) - 1);

				int whileLoopLimit = 0;

				//If the currently selected tile can be overriden, do that, if not, find another tile. Repeat 10 time. if failed every time, give up.
				while (FloorValuesArray[keyTileIndex].FloorObstacle > EFloorObstacle::Basic)
				{
					keyTileIndex = FMath::RandRange(0, (DepthRange.Final * WidthRange.Final) - 1);

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

					FindStartToEndPath(startingIndex, keyTileIndex, WidthRange.Final);
				}
			}

			//Override previously initialized tiles with survival zone tiles
			for (int i = 0; i < MaxSurvivalZoneAmount; i++)
			{
				int keyTileIndex = FMath::RandRange(0, (DepthRange.Final * WidthRange.Final) - 1);

				int whileLoopLimit = 0;

				//If the currently selected tile can be overriden, do that, if not, find another tile. Repeat 10 time. if failed every time, give up.
				while (FloorValuesArray[keyTileIndex].FloorObstacle > EFloorObstacle::Basic)
				{
					keyTileIndex = FMath::RandRange(0, (DepthRange.Final * WidthRange.Final) - 1);

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

					FindStartToEndPath(startingIndex, keyTileIndex, WidthRange.Final);
				}
			}

			//Override previously initialized tiles with Vantage Point tiles
			for (int i = 0; i < MaxVantagePointActors; i++)
			{
				int keyTileIndex = FMath::RandRange(0, (DepthRange.Final * WidthRange.Final) - 1);

				int whileLoopLimit = 0;

				//If the currently selected tile can be overriden, do that, if not, find another tile. Repeat 10 time. if failed every time, give up.
				while (FloorValuesArray[keyTileIndex].FloorObstacle > EFloorObstacle::Vantage)
				{
					keyTileIndex = FMath::RandRange(0, (DepthRange.Final * WidthRange.Final) - 1);

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

					FindStartToEndPath(startingIndex, keyTileIndex, WidthRange.Final);
				}
			}

			if (!FloorValuesArray.IsEmpty())
			{
				for (int y = 0; y < DepthRange.Final; y++)
				{
					for (int x = 0; x < WidthRange.Final; x++)
					{
						//Spawn the floor tiles with the instanced static mesh component.
						CheckFloor(x, y, WidthRange.Final, DepthRange.Final);

						//Spawn each modular obstacle based on previously defined variable.
						Server_SpawnFloorObstacles(x, y, WidthRange.Final);
					}
				}
				//Spawn the floor with the instanced static mesh component.
				CreateFloor();
			}
		}

		//Set the gamestate's objective type in order to help keep track of remaining objectives if it is set to anything.
		ARelicRunnersGameState* gameState = Cast<ARelicRunnersGameState>(GetWorld()->GetGameState());
		if (gameState)
		{
			gameState->Server_SetObjectiveType(ObjectiveType);
		}
	}

	TArray<FString> enumName;

	if (EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::CaptureTheFlag))
		enumName.Add(TEXT("Capture the Flag"));
	if (EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::DefeatAllEnemies))
		enumName.Add(TEXT("Defeat all anemies"));
	if (EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::Survival))
		enumName.Add(TEXT("Survival"));
	if (EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::MoveThePayload))
		enumName.Add(TEXT("Deliver package"));

	for (FString& value : enumName)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Current Objective:")) + value);
	}
}

void ALevelGenerator::BeginPlay()
{
	Super::BeginPlay();

	//Navmesh stuff. Thanks Konstantin
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		NavSys->Build();
	}
}


void ALevelGenerator::GenerateFloor(int x, int y)
{
	if (FloorValuesArray.Num() == x + (WidthRange.Final * y))
	{
		//Set current tile as walkable or non-walkable
		InitializeFloor();

		//If initialized, force radius of tiles in the center to walkable
		if (CenterForceFull > 0)
		{
			if (((x - CenterForceFull) >= 0 && (x + CenterForceFull) <= WidthRange.Final - 1)
				&& (y - CenterForceFull) >= 0 && (y + CenterForceFull) <= DepthRange.Final - 1)
			{
				ForceFloorBool(true, x, y, WidthRange.Final);
			}
		}

		//If initialized, force border to walkable
		if (BorderForceFull > 0)
		{
			if (((x - BorderForceFull) < 0 || (x + BorderForceFull) > WidthRange.Final - 1)
				|| (y - BorderForceFull) < 0 || (y + BorderForceFull) > DepthRange.Final - 1)

			{
				ForceFloorBool(true, x, y, WidthRange.Final);
			}
		}
	}
}

bool ALevelGenerator::CalculateDistanceBetweenTiles(int firstIndex, int secondIndex) const
{
	if ((StartingToEndingMinDistance + (StartingToEndingMinDistance * WidthRange.Final)) > WidthRange.Final * DepthRange.Final)
	{
		return false;
	}

	for (int y = -StartingToEndingMinDistance; y < StartingToEndingMinDistance; y++)
	{
		for (int x = -StartingToEndingMinDistance; x < StartingToEndingMinDistance; x++)
		{
			int currentX = firstIndex % WidthRange.Final + x;
			int currentY = firstIndex / WidthRange.Final + y;

			bool validX = currentX >= 0 && currentX < WidthRange.Final;
			bool validY = currentY >= 0 && currentY < DepthRange.Final;

			if (validX && validY)
			{
				if ((currentX + (currentY * WidthRange.Final)) == secondIndex)
				{
					return true;
				}
			}
		}
	}
	return false;
}

void ALevelGenerator::InitializeFloor()
{
	//Based on the percentage, set floor tile to walkable or non-walkable.
	FSFloorValues newFloorValues;
	float randomFloor = FMath::RandRange(0.1f, 100.0f);

	if (randomFloor > FullPercentage)
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

void ALevelGenerator::ForceFloorBool(bool forcedFloor, int x, int y, int width)
{
	int index = x + (y * WidthRange.Final);

	if (!EnumHasAnyFlags(FloorValuesArray[index].FloorObstacle, EFloorObstacle::Hole))
	{
		FloorValuesArray[index].IsFullTile = forcedFloor;

		if (forcedFloor)
		{
			if (FloorValuesArray[index].FloorObstacle == EFloorObstacle::None)
			{
				InitializeModularObstacle(FloorValuesArray[index], false);
			}
		}
	}
}

void ALevelGenerator::SetStartingAndEndingPoints(int startingIndex, int endingIndex)
{
	FloorValuesArray[startingIndex].IsFullTile = true;
	FloorValuesArray[startingIndex].FloorObstacle = EFloorObstacle::Start;
	FloorValuesArray[startingIndex].randomFloorToSpawn = 0;
	FloorValuesArray[startingIndex].obstacleYaw = FMath::RandRange(0, 3) * 90;


	FloorValuesArray[endingIndex].IsFullTile = true;
	FloorValuesArray[endingIndex].FloorObstacle = EFloorObstacle::End;
	FloorValuesArray[endingIndex].randomFloorToSpawn = 0;
	FloorValuesArray[endingIndex].obstacleYaw = FMath::RandRange(0, 3) * 90;
}

void ALevelGenerator::SetFlagTile(int index)
{
	FloorValuesArray[index].IsFullTile = true;
	FloorValuesArray[index].FloorObstacle = EFloorObstacle::CapturableFlag;
	FloorValuesArray[index].randomFloorToSpawn = 0;
	FloorValuesArray[index].obstacleYaw = FMath::RandRange(0, 3) * 90;

}

void ALevelGenerator::SetEnemyZoneTile(int index)
{
	FloorValuesArray[index].IsFullTile = true;
	FloorValuesArray[index].FloorObstacle = EFloorObstacle::EnemyZone;
	FloorValuesArray[index].randomFloorToSpawn = 0;
	FloorValuesArray[index].obstacleYaw = FMath::RandRange(0, 3) * 90;
}

void ALevelGenerator::SetSurvivalZoneTile(int index)
{
	FloorValuesArray[index].IsFullTile = true;
	FloorValuesArray[index].FloorObstacle = EFloorObstacle::SurvivalZone;
	FloorValuesArray[index].randomFloorToSpawn = 0;
	FloorValuesArray[index].obstacleYaw = FMath::RandRange(0, 3) * 90;
}

void ALevelGenerator::SetVantagePointTile(int index)
{
	FloorValuesArray[index].IsFullTile = true;
	FloorValuesArray[index].FloorObstacle = EFloorObstacle::Vantage;
	FloorValuesArray[index].randomFloorToSpawn = 0;
	FloorValuesArray[index].obstacleYaw = FMath::RandRange(0, 3) * 90;
}

void ALevelGenerator::FindStartToEndPath(int startingIndex, int targetIndex, int width)
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

void ALevelGenerator::FindStartToEndPath(int startingIndex, int targetIndex, int width, bool implementSpline)
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

		bReachEndX = currentX == targetX;
		bReachEndY = currentY == targetY;
		if (!(bReachEndX && bReachEndY))
		{
			ForceFloorBool(true, currentX, currentY, width);

			if (implementSpline && bValueHasChanged && EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::MoveThePayload))
			{
				DeliverySpline->AddSplinePoint(FVector((TileScale * currentX * 4) + TileScale, (TileScale * currentY * 4) + TileScale, 0.0f), ESplineCoordinateSpace::World);
				bValueHasChanged = false;
			}
		}
	}
}

void ALevelGenerator::InitializeModularObstacle(FSFloorValues& floorValue, bool canGenerateHoles) const
{
	float holeTilePercentage = FMath::RandRange(0.1f, 100.0f);

	if (holeTilePercentage < HolePercentage)
	{
		if (canGenerateHoles)
		{
			floorValue.FloorObstacle = EFloorObstacle::Hole;

			floorValue.randomFloorToSpawn = FMath::RandRange(0, HoleTilePackedActorArray.Num() - 1);

			floorValue.obstacleYaw = FMath::RandRange(0, 3) * 90;

			floorValue.IsFullTile = false;
		}
		else
		{
			float randomObstaclePerc = FMath::RandRange(0.1f, 100.0f);
			if (randomObstaclePerc < BasicObstaclePercentage)
			{
				floorValue.FloorObstacle = EFloorObstacle::Basic;

				floorValue.randomFloorToSpawn = FMath::RandRange(0, PackedActorArray.Num() - 1);

				floorValue.obstacleYaw = FMath::RandRange(0, 3) * 90;
			}
			else
			{
				floorValue.FloorObstacle = EFloorObstacle::None;
			}
		}
	}
	else
	{
		float randomObstaclePerc = FMath::RandRange(0.1f, 100.0f);
		if (randomObstaclePerc < BasicObstaclePercentage)
		{
			floorValue.FloorObstacle = EFloorObstacle::Basic;

			floorValue.randomFloorToSpawn = FMath::RandRange(0, PackedActorArray.Num() - 1);

			floorValue.obstacleYaw = FMath::RandRange(0, 3) * 90;
		}
	}
}

void ALevelGenerator::CheckFloor(int x, int y, int width, int depth)
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

void ALevelGenerator::FloorCheckTopNeighbours(bool checkLeft, bool checkRight, int indexOffset, int x, int y, int width)
{
	int indexToCheck = x + (y * width) + indexOffset;
	int index = x + (y * width);

	if (checkLeft)
	{
		if (FloorValuesArray[indexToCheck-1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EFloorNeighbours::TopLeft;
			FloorValuesArray[index].ObstacleNeighbours |= EFloorNeighbours::TopLeft;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EFloorNeighbours::TopLeft;
		}

		if (FloorValuesArray[indexToCheck - 1].FloorObstacle == EFloorObstacle::Hole)
		{
			FloorValuesArray[index].ObstacleNeighbours |= EFloorNeighbours::TopLeft;
		}
	}

	if (FloorValuesArray[indexToCheck].IsFullTile)
	{
		FloorValuesArray[index].FloorNeighbours |= EFloorNeighbours::TopMiddle;
		FloorValuesArray[index].ObstacleNeighbours |= EFloorNeighbours::TopMiddle;
	}
	else
	{
		FloorValuesArray[index].FloorNeighbours &= ~EFloorNeighbours::TopMiddle;
	}

	if (FloorValuesArray[indexToCheck].FloorObstacle == EFloorObstacle::Hole)
	{
		FloorValuesArray[index].ObstacleNeighbours |= EFloorNeighbours::TopMiddle;
	}

	if (checkRight)
	{
		if (FloorValuesArray[indexToCheck + 1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EFloorNeighbours::TopRight;
			FloorValuesArray[index].ObstacleNeighbours |= EFloorNeighbours::TopRight;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EFloorNeighbours::TopRight;
		}

		if (FloorValuesArray[indexToCheck + 1].FloorObstacle == EFloorObstacle::Hole)
		{
			FloorValuesArray[index].ObstacleNeighbours |= EFloorNeighbours::TopRight;
		}
	}
}

void ALevelGenerator::FloorCheckMiddleNeighbours(bool checkLeft, bool checkRight, int indexOffset, int x, int y, int width)
{
	int indexToCheck = x + (y * width) + indexOffset;
	int index = x + (y * width);

	if (checkLeft)
	{
		if (FloorValuesArray[indexToCheck - 1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EFloorNeighbours::MiddleLeft;
			FloorValuesArray[index].ObstacleNeighbours |= EFloorNeighbours::MiddleLeft;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EFloorNeighbours::MiddleLeft;
		}
		
		if (FloorValuesArray[indexToCheck - 1].FloorObstacle == EFloorObstacle::Hole)
		{
			FloorValuesArray[index].ObstacleNeighbours |= EFloorNeighbours::MiddleLeft;
		}
	}

	if (checkRight)
	{
		if (FloorValuesArray[indexToCheck + 1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EFloorNeighbours::MiddleRight;
			FloorValuesArray[index].ObstacleNeighbours |= EFloorNeighbours::MiddleRight;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EFloorNeighbours::MiddleRight;
		}

		if (FloorValuesArray[indexToCheck + 1].FloorObstacle == EFloorObstacle::Hole)
		{
			FloorValuesArray[index].ObstacleNeighbours |= EFloorNeighbours::MiddleRight;
		}
	}

}

void ALevelGenerator::FloorCheckBottomNeighbours(bool checkLeft, bool checkRight, int indexOffset, int x, int y, int width)
{
	int indexToCheck = x + (y * width) + indexOffset;
	int index = x + (y * width);

	if (checkLeft)
	{
		if (FloorValuesArray[indexToCheck - 1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EFloorNeighbours::BottomLeft;
			FloorValuesArray[index].ObstacleNeighbours |= EFloorNeighbours::BottomLeft;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EFloorNeighbours::BottomLeft;
		}

		if (FloorValuesArray[indexToCheck - 1].FloorObstacle == EFloorObstacle::Hole)
		{
			FloorValuesArray[index].ObstacleNeighbours |= EFloorNeighbours::BottomLeft;
		}
	}

	if (FloorValuesArray[indexToCheck].IsFullTile)
	{
		FloorValuesArray[index].FloorNeighbours |= EFloorNeighbours::BottomMiddle;
		FloorValuesArray[index].ObstacleNeighbours |= EFloorNeighbours::BottomMiddle;
	}
	else
	{
		FloorValuesArray[index].FloorNeighbours &= ~EFloorNeighbours::BottomMiddle;
	}

	if (FloorValuesArray[indexToCheck].FloorObstacle == EFloorObstacle::Hole)
	{
		FloorValuesArray[index].ObstacleNeighbours |= EFloorNeighbours::BottomMiddle;
	}

	if (checkRight)
	{
		if (FloorValuesArray[indexToCheck + 1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EFloorNeighbours::BottomRight;
			FloorValuesArray[index].ObstacleNeighbours |= EFloorNeighbours::BottomRight;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EFloorNeighbours::BottomRight;
		}

		if (FloorValuesArray[indexToCheck + 1].FloorObstacle == EFloorObstacle::Hole)
		{
			FloorValuesArray[index].ObstacleNeighbours |= EFloorNeighbours::BottomRight;
		}
	}
}

void ALevelGenerator::SetFloorShape(int x, int y, int width)
{
	SetTopLeftFloorShape(x, y, width);
	SetTopRightFloorShape(x, y, width);
	SetBottomLeftFloorShape(x, y, width);
	SetBottomRightFloorShape(x, y, width);
}

void ALevelGenerator::SetTopLeftFloorShape(int x, int y, int width)
{
	int index = x + (y * width);
	FVector posOffset = FVector((x * 2), (y * 2), 0.0f) * (TileScale * 2);
	FRotator rotationOffset = FRotator(0.0f, 0.0f, 0.0f);
	float scaleOffset = TileScale * 2;
	FTransform newTransform;

	newTransform.SetLocation(posOffset);
	newTransform.SetRotation(rotationOffset.Quaternion());
	newTransform.SetScale3D(FVector(scaleOffset));

	EFloorNeighbours topLeftCornerCheck = FloorValuesArray[index].FloorNeighbours & (EFloorNeighbours::TopLeft | EFloorNeighbours::TopMiddle | EFloorNeighbours::MiddleLeft);

	if (FloorValuesArray[index].IsFullTile)
	{
		switch (topLeftCornerCheck)
		{
		case EFloorNeighbours::TopMiddle:
		case EFloorNeighbours::TopMiddle | EFloorNeighbours::TopLeft:
			SideFloorPieceTransforms.Add(newTransform);
			break;
		case EFloorNeighbours::MiddleLeft:
		case EFloorNeighbours::MiddleLeft | EFloorNeighbours::TopLeft:
			rotationOffset.Yaw = 90.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			SideFloorPieceTransforms.Add(newTransform);
			break;
		case EFloorNeighbours::TopMiddle | EFloorNeighbours::MiddleLeft:
		case EFloorNeighbours::TopLeft | EFloorNeighbours::TopMiddle | EFloorNeighbours::MiddleLeft:
			FullFloorPieceTransforms.Add(newTransform);
			break;
		case EFloorNeighbours::TopLeft:
		default:
			ConvexCornerTransforms.Add(newTransform);
		}
	}
}

void ALevelGenerator::SetTopRightFloorShape(int x, int y, int width)
{
	int index = x + (y * width);
	FVector posOffset = FVector((x * 2) + 1.0f, (y * 2), 0.0f) * (TileScale * 2);
	FRotator rotationOffset = FRotator(0.0f, 0.0f, 0.0f);
	float scaleOffset = TileScale * 2;
	FTransform newTransform;

	newTransform.SetLocation(posOffset);
	newTransform.SetRotation(rotationOffset.Quaternion());
	newTransform.SetScale3D(FVector(scaleOffset));

	EFloorNeighbours topRightCornerCheck = FloorValuesArray[index].FloorNeighbours & (EFloorNeighbours::TopRight | EFloorNeighbours::TopMiddle | EFloorNeighbours::MiddleRight);

	if (FloorValuesArray[index].IsFullTile)
	{
		switch (topRightCornerCheck)
		{
		case EFloorNeighbours::TopMiddle:
		case EFloorNeighbours::TopMiddle | EFloorNeighbours::TopRight:
			rotationOffset.Yaw = 180.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			SideFloorPieceTransforms.Add(newTransform);
			break;
		case EFloorNeighbours::MiddleRight:
		case EFloorNeighbours::MiddleRight | EFloorNeighbours::TopRight:
			rotationOffset.Yaw = 90.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			SideFloorPieceTransforms.Add(newTransform);
			break;
		case EFloorNeighbours::TopMiddle | EFloorNeighbours::MiddleRight:
		case EFloorNeighbours::TopRight | EFloorNeighbours::TopMiddle | EFloorNeighbours::MiddleRight:
			FullFloorPieceTransforms.Add(newTransform);
			break;
		case EFloorNeighbours::TopRight:
		default:
			rotationOffset.Yaw = 90.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			ConvexCornerTransforms.Add(newTransform);
			break;
		}

	}
}

void ALevelGenerator::SetBottomLeftFloorShape(int x, int y, int width)
{
	int index = x + (y * width);
	FVector posOffset = FVector((x * 2), (y * 2) + 1.0f, 0.0f) * (TileScale * 2);
	FRotator rotationOffset = FRotator(0.0f, 0.0f, 0.0f);
	float scaleOffset = TileScale * 2;
	FTransform newTransform;

	newTransform.SetLocation(posOffset);
	newTransform.SetRotation(rotationOffset.Quaternion());
	newTransform.SetScale3D(FVector(scaleOffset));

	EFloorNeighbours bottomLeftCornerCheck = FloorValuesArray[index].FloorNeighbours & (EFloorNeighbours::BottomLeft | EFloorNeighbours::BottomMiddle | EFloorNeighbours::MiddleLeft);

	if (FloorValuesArray[index].IsFullTile)
	{
		switch (bottomLeftCornerCheck)
		{
		case EFloorNeighbours::BottomMiddle:
		case EFloorNeighbours::BottomMiddle | EFloorNeighbours::BottomLeft:
			SideFloorPieceTransforms.Add(newTransform);
			break;
		case EFloorNeighbours::MiddleLeft:
		case EFloorNeighbours::MiddleLeft | EFloorNeighbours::BottomLeft:
			rotationOffset.Yaw = -90.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			SideFloorPieceTransforms.Add(newTransform);
			break;
		case EFloorNeighbours::BottomMiddle | EFloorNeighbours::MiddleLeft:
		case EFloorNeighbours::BottomLeft | EFloorNeighbours::BottomMiddle | EFloorNeighbours::MiddleLeft:
			FullFloorPieceTransforms.Add(newTransform);
			break;
		case EFloorNeighbours::BottomLeft:
		default:
			rotationOffset.Yaw = -90.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			ConvexCornerTransforms.Add(newTransform);
			break;
		}
	}
}

void ALevelGenerator::SetBottomRightFloorShape(int x, int y, int width)
{
	int index = x + (y * width);
	FVector posOffset = FVector((x * 2) + 1.0f, (y * 2) + 1.0f, 0.0f) * (TileScale * 2);
	FRotator rotationOffset = FRotator(0.0f, 0.0f, 0.0f);
	float scaleOffset = TileScale * 2;
	FTransform newTransform;

	newTransform.SetLocation(posOffset);
	newTransform.SetRotation(rotationOffset.Quaternion());
	newTransform.SetScale3D(FVector(scaleOffset));

	EFloorNeighbours bottomRightCornerCheck = FloorValuesArray[index].FloorNeighbours & (EFloorNeighbours::BottomRight | EFloorNeighbours::BottomMiddle | EFloorNeighbours::MiddleRight);

	if (FloorValuesArray[index].IsFullTile)
	{
		switch (bottomRightCornerCheck)
		{
		case EFloorNeighbours::BottomMiddle:
		case EFloorNeighbours::BottomMiddle | EFloorNeighbours::BottomRight:
			rotationOffset.Yaw = 180.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			SideFloorPieceTransforms.Add(newTransform);
			break;
		case EFloorNeighbours::MiddleRight:
		case EFloorNeighbours::MiddleRight | EFloorNeighbours::BottomRight:
			rotationOffset.Yaw = -90.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			SideFloorPieceTransforms.Add(newTransform);
			break;
		case EFloorNeighbours::BottomMiddle | EFloorNeighbours::MiddleRight:
		case EFloorNeighbours::BottomRight | EFloorNeighbours::BottomMiddle | EFloorNeighbours::MiddleRight:
			FullFloorPieceTransforms.Add(newTransform);
			break;
		case EFloorNeighbours::BottomRight:
		default:
			rotationOffset.Yaw = 180.0f;
			newTransform.SetRotation(rotationOffset.Quaternion());
			ConvexCornerTransforms.Add(newTransform);
			break;
		}
	}
}

void ALevelGenerator::Server_SpawnFloorObstacles_Implementation(int x, int y, int width)
{
	SpawnFloorObstacles(x, y, width);
}

void ALevelGenerator::SpawnFloorObstacles(int x, int y, int width)
{
	int index = x + (y * width);

	float yaw = FloorValuesArray[index].obstacleYaw;
	FVector posOffset = FVector((TileScale * x * 4) + TileScale, (TileScale * y * 4) + TileScale, 0.0f);

	switch (FloorValuesArray[index].FloorObstacle)
	{
	case EFloorObstacle::Basic:
		if (PackedActorArray.IsValidIndex(FloorValuesArray[index].randomFloorToSpawn))
		{
			APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorArray[FloorValuesArray[index].randomFloorToSpawn], posOffset, FRotator(0.0f, yaw, 0.0f));
		}

		if (PackedActorBasicWall != nullptr)
		{
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 0.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 90.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 180.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 270.0f, 0.0f));
			}
		}
		break;
	case EFloorObstacle::Hole:
		if (HoleTilePackedActorArray.IsValidIndex(FloorValuesArray[index].randomFloorToSpawn))
		{
			APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(HoleTilePackedActorArray[FloorValuesArray[index].randomFloorToSpawn], posOffset, FRotator(0.0f, yaw, 0.0f));
		}

		if (HoleTilePackedActorWallArray.IsValidIndex(FloorValuesArray[index].randomFloorToSpawn))
		{
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(HoleTilePackedActorWallArray[FloorValuesArray[index].randomFloorToSpawn], posOffset, FRotator(0.0f, 0.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(HoleTilePackedActorWallArray[FloorValuesArray[index].randomFloorToSpawn], posOffset, FRotator(0.0f, 90.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(HoleTilePackedActorWallArray[FloorValuesArray[index].randomFloorToSpawn], posOffset, FRotator(0.0f, 180.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(HoleTilePackedActorWallArray[FloorValuesArray[index].randomFloorToSpawn], posOffset, FRotator(0.0f, 270.0f, 0.0f));
			}
		}
		break;
	case EFloorObstacle::Vantage:
		if (!VantagePackedActorArray.IsEmpty())
		{
			int randIndex = FMath::RandRange(0, VantagePackedActorArray.Num() - 1);

			if (VantagePackedActorArray.IsValidIndex(randIndex))
			{
				if (VantagePackedActorArray[randIndex])
				{
					APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(VantagePackedActorArray[randIndex], posOffset, FRotator(0.0f, yaw, 0.0f));
				}
			}

			if (PackedActorBasicWall != nullptr)
			{
				if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
				{
					APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 0.0f, 0.0f));
				}
				if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
				{
					APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 90.0f, 0.0f));
				}
				if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
				{
					APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 180.0f, 0.0f));
				}
				if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
				{
					APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 270.0f, 0.0f));
				}
			}
		}
		break;

	case EFloorObstacle::CapturableFlag:
		if (EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::CaptureTheFlag))
		{
			if (!CapturableFlagPackedActorArray.IsEmpty())
			{
				int randIndex = FMath::RandRange(0, CapturableFlagPackedActorArray.Num() - 1);

				if (CapturableFlagPackedActorArray.IsValidIndex(randIndex))
				{
					APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(CapturableFlagPackedActorArray[randIndex], posOffset, FRotator(0.0f, yaw, 0.0f));
				}
			}

			if (PackedActorBasicWall != nullptr)
			{
				if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
				{
					APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 0.0f, 0.0f));
				}
				if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
				{
					APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 90.0f, 0.0f));
				}
				if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
				{
					APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 180.0f, 0.0f));
				}
				if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
				{
					APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 270.0f, 0.0f));
				}
			}

		}
		break;
	case EFloorObstacle::EnemyZone:
		if (EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::DefeatAllEnemies))
		{
			if (!EnemyZonePackedActorArray.IsEmpty())
			{
				int randIndex = FMath::RandRange(0, EnemyZonePackedActorArray.Num() - 1);

				if (EnemyZonePackedActorArray.IsValidIndex(randIndex))
				{
					APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(EnemyZonePackedActorArray[randIndex], posOffset, FRotator(0.0f, yaw, 0.0f));
				}
			}

			if (PackedActorBasicWall != nullptr)
			{
				if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
				{
					APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 0.0f, 0.0f));
				}
				if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
				{
					APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 90.0f, 0.0f));
				}
				if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
				{
					APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 180.0f, 0.0f));
				}
				if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
				{
					APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 270.0f, 0.0f));
				}
			}

		}
		break;
	case EFloorObstacle::SurvivalZone:
		if (EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::Survival))
		{
			if (!SurvivalZonePackedActorArray.IsEmpty())
			{
				int randIndex = FMath::RandRange(0, SurvivalZonePackedActorArray.Num() - 1);

				if (SurvivalZonePackedActorArray.IsValidIndex(randIndex))
				{
					APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(SurvivalZonePackedActorArray[randIndex], posOffset, FRotator(0.0f, yaw, 0.0f));
				}
			}

			if (PackedActorBasicWall != nullptr)
			{
				if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
				{
					APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 0.0f, 0.0f));
				}
				if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
				{
					APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 90.0f, 0.0f));
				}
				if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
				{
					APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 180.0f, 0.0f));
				}
				if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
				{
					APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 270.0f, 0.0f));
				}
			}

		}
		break;
	case EFloorObstacle::Start:
		if (LevelStartPackedActor != nullptr)
		{
			APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(LevelStartPackedActor, posOffset, FRotator(0.0f, yaw, 0.0f));
		}

		if (EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::MoveThePayload))
		{
			DeliverySpline->SetLocationAtSplinePoint(0, posOffset, ESplineCoordinateSpace::World);
		}

		if (PackedActorBasicWall != nullptr)
		{
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 0.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 90.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 180.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 270.0f, 0.0f));
			}
		}

		break;
	case EFloorObstacle::End:
		if (LevelEndPackedActor != nullptr)
		{
			APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(LevelEndPackedActor, posOffset, FRotator(0.0f, yaw, 0.0f));
		}

		if (LevelChangeTrigger != nullptr)
		{
			ALevelChangeTrigger* levelChangeActor = GetWorld()->SpawnActor<ALevelChangeTrigger>(LevelChangeTrigger, posOffset, FRotator(0.0f, yaw, 0.0f));

			if (!TargetLevel.IsNull())
			{
				levelChangeActor->Server_SetTargetLevel(FName(TargetLevel.GetAssetName()));
			}
		}

		if (PackedActorBasicWall != nullptr)
		{
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 0.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 90.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 180.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 270.0f, 0.0f));
			}
		}

		break;
	case EFloorObstacle::None:

		if (PackedActorBasicWall != nullptr && FloorValuesArray[index].IsFullTile)
		{
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleLeft))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 0.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::TopMiddle))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 90.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::MiddleRight))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 180.0f, 0.0f));
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EFloorNeighbours::BottomMiddle))
			{
				APackedLevelActor* wall = GetWorld()->SpawnActor<APackedLevelActor>(PackedActorBasicWall, posOffset, FRotator(0.0f, 270.0f, 0.0f));
			}
		}
		break;
	}
}

void ALevelGenerator::Server_SpawnFloorObstaclesByColour_Implementation(int x, int y, int width, FColor colour)
{
	SpawnFloorObstaclesByColour(x, y, width, colour);
}

void ALevelGenerator::SpawnFloorObstaclesByColour(int x, int y, int width, FColor colour)
{
	if (TileColourToPackedActor.Find(colour) != nullptr)
	{
		FVector posOffset = FVector((TileScale * x * 4) + TileScale, (TileScale * y * 4) + TileScale, TileScale);
		float obstacleYaw = TileColourToPackedActor.Find(colour)->IsRandomYaw ?
			//True
			FMath::RandRange(0, 3) * 90 :
			//False
			TileColourToPackedActor.Find(colour)->ObstacleYaw * 90;

		APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(TileColourToPackedActor.Find(colour)->ObstacleActor, posOffset, FRotator(0.0f, obstacleYaw, 0.0f));

		if (TileColourToPackedActor.Find(colour)->FloorObstacle != EFloorObstacle::End)
			return;

		if (LevelChangeTrigger == nullptr)
			return;

		ALevelChangeTrigger* levelChangeActor = GetWorld()->SpawnActor<ALevelChangeTrigger>(LevelChangeTrigger, posOffset, FRotator(0.0f, obstacleYaw, 0.0f));

		if (TargetLevel.IsNull())
			return;

		levelChangeActor->Server_SetTargetLevel(FName(TargetLevel.GetAssetName()));
	}
}

void ALevelGenerator::CreateFloor()
{
	TArray<TArray<FTransform>> arrayOfTransformArrays;
	arrayOfTransformArrays.Add(FullFloorPieceTransforms);
	arrayOfTransformArrays.Add(SideFloorPieceTransforms);
	arrayOfTransformArrays.Add(ConvexCornerTransforms);

	TArray<UStaticMesh*> staticMeshArray;
	staticMeshArray.Add(FullFloorMesh);
	staticMeshArray.Add(SideFloorMesh);
	staticMeshArray.Add(ConvexCornerMesh);

	for (int i = 0; i < staticMeshArray.Num(); i++)
	{
		UInstancedStaticMeshComponent* ISMComp = NewObject<UInstancedStaticMeshComponent>(this);
					
		if (staticMeshArray[i] != nullptr)
		{
			ISMComp->SetStaticMesh(staticMeshArray[i]);
		}
		if (FloorMaterial != nullptr)
		{
			ISMComp->SetMaterial(0, FloorMaterial);
		}

		ISMComp->AddInstances(arrayOfTransformArrays[i], false, true, true);

		ISMComp->RegisterComponent();
	}
}

void ALevelGenerator::OnRep_FloorValuesArrayChange()
{
	for (int y = 0; y < DepthRange.Final; y++)
	{
		for (int x = 0; x < DepthRange.Final; x++)
		{
			CheckFloor(x, y, WidthRange.Final, DepthRange.Final);
		}
	}
	CreateFloor();
}

void ALevelGenerator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALevelGenerator, FloorValuesArray);
}