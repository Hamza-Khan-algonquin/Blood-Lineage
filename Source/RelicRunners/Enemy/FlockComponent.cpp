#include "FlockComponent.h"
#include "EnemyBoidMovementComponent.h"
#include <Net/UnrealNetwork.h>

UFlockComponent::UFlockComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	SetIsReplicatedByDefault(true);
	MoveDataArray.Owner = this;
}

void UFlockComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(UFlockComponent, MoveDataArray);
	DOREPLIFETIME(UFlockComponent, BoidMovementComponents);
}

void UFlockComponent::AddBoid(UEnemyBoidMovementComponent* boid)
{
	BoidMovementComponents.Add(boid);
	FMoveData& moveData = MoveDataArray.MoveDatas.AddDefaulted_GetRef();
	moveData.BoidID = boid->GetUniqueID();
	boid->ServerSideID = moveData.BoidID;
	moveData.Location = boid->UpdatedComponent->GetComponentLocation();
	moveData.Rotation = boid->UpdatedComponent->GetComponentRotation();
	MoveDataArray.MarkItemDirty(moveData);
}

void UFlockComponent::RemoveBoid(UEnemyBoidMovementComponent* boid)
{
	BoidMovementComponents.Remove(boid);

	//for what ever reason it was giving me a error when I used a for each loop
	for (int i = 0; i < MoveDataArray.MoveDatas.Num(); i++)
	{
		if (MoveDataArray.MoveDatas[i].BoidID == boid->ServerSideID)
		{
			MoveDataArray.MoveDatas.RemoveAt(i);
			MoveDataArray.MarkArrayDirty();
			return;
		}
	}
}

void UFlockComponent::UpdateBoidMoveData(float delta)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	//using a map as otherwise we would have to loop through the array everytime
	TMap<int32, UEnemyBoidMovementComponent*> idToBoid;

	for (UEnemyBoidMovementComponent* boid : BoidMovementComponents)
	{
		if (IsValid(boid))
		{
			idToBoid.Add(boid->ServerSideID, boid);
		}
	}

	for (FMoveData& Data : MoveDataArray.MoveDatas)
	{
		if (UEnemyBoidMovementComponent** boidPtr = idToBoid.Find(Data.BoidID))
		{
			UEnemyBoidMovementComponent* boid = *boidPtr;

			if (!IsValid(boid) || !IsValid(boid->UpdatedComponent))
			{
				continue;
			}

			if (boid->UpdatedComponent->GetComponentLocation() != Data.Location || boid->UpdatedComponent->GetComponentRotation() != Data.Rotation)
			{
				Data.Location = boid->UpdatedComponent->GetComponentLocation();
				Data.Rotation = boid->UpdatedComponent->GetComponentRotation();
				MoveDataArray.MarkItemDirty(Data);
			}
		}
	}
}


// Called when the game starts
void UFlockComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UFlockComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateBoidMoveData(DeltaTime);
}

void FMoveData::PostReplicatedAdd(const FMoveDataArray& InArraySerializer)
{
	UE_LOG(LogTemp, Warning, TEXT("PostReplicatedAdd called for BoidID: %d"), this->BoidID);
	for (UEnemyBoidMovementComponent* boid : InArraySerializer.Owner->BoidMovementComponents)
	{
		if (!IsValid(boid))
		{
			continue;
		}

		if (this->BoidID == boid->ServerSideID)
		{
			boid->AddToMoveDataBuffer(this->Location, this->Rotation);
			return;
		}
	}
}

void FMoveData::PostReplicatedChange(const FMoveDataArray & InArraySerializer)
{
	UE_LOG(LogTemp, Warning, TEXT("PostReplicatedChange called for BoidID: %d"), this->BoidID);
	for (UEnemyBoidMovementComponent* boid : InArraySerializer.Owner->BoidMovementComponents)
	{
		if (!IsValid(boid))
		{
			continue;
		}

		if (this->BoidID == boid->ServerSideID)
		{
			boid->AddToMoveDataBuffer(this->Location, this->Rotation);
			return;
		}
	}
}
