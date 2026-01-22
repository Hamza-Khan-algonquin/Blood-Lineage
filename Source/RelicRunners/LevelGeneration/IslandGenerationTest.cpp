// Fill out your copyright notice in the Description page of Project Settings.


#include "IslandGenerationTest.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "IslandSpawningValuesComponent.h"
#include "Net/UnrealNetwork.h"

/*
Set index of starting tile and ending (bridge) tile

Initialize values of each floor tile based on width and depth + set what regular obstacle will spawn based on a precentage

Override previously defined tiles with starting and ending tiles

Based on maximum amount, override previously defined tiles with the objective of that island
Spawn the objective at the appropriate location

Spawn obstacles and 

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
			
		for (int y = 0; y < IslandDepth; y++)
		{
			for (int x = 0; x < IslandWidth; x++)
			{
				//Initialize each tile and set them to either walkable or non-walkable.
				GenerateFloor(x, y);
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
				}
			}
			//Spawn the floor with the instanced static mesh component.
			CreateFloor();
		}
	}
}

void AIslandGenerationTest::InitializeValues()
{
	IslandDepth = FMath::RandRange(GenerationValuesComponent->IslandValues.DepthMin, GenerationValuesComponent->IslandValues.DepthMax);
	IslandWidth = FMath::RandRange(GenerationValuesComponent->IslandValues.WidthMin, GenerationValuesComponent->IslandValues.WidthMax);
}

void AIslandGenerationTest::GenerateFloor(int x, int y)
{
	if (FloorValuesArray.Num() == x + (IslandWidth * y))
	{
		//Set current tile as walkable or non-walkable
		InitializeFloor();
	}
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
	}

	//Add initialized floor to array
	FloorValuesArray.Add(newFloorValues);
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