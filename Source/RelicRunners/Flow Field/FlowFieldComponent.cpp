// Fill out your copyright notice in the Description page of Project Settings.


#include "FlowFieldComponent.h"
#include "NavMesh/RecastNavMesh.h"
#include "Detour/DetourNavMeshQuery.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastQueryFilter.h"
#include "NavFilters/NavigationQueryFilter.h"

// Sets default values for this component's properties
UFlowFieldComponent::UFlowFieldComponent()
{
#if WITH_EDITOR
	DrawFlowField = false;
#endif

	FieldRadius = 10000.f;

	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UFlowFieldComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UNavigationSystemV1* navSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!navSys)
	{
		return;
	}

	navSys->OnNavigationGenerationFinishedDelegate.RemoveDynamic(this, &UFlowFieldComponent::CheckForInvalidNodeRefs);
	Super::EndPlay(EndPlayReason);
}

FVector UFlowFieldComponent::GetDirection(FVector location, FVector extent)
{
	FNavLocation navLocation;

	ActiveNavMesh->ProjectPoint(location, navLocation, extent);
	if (FlowField.Contains(navLocation.NodeRef))
	{
		return FlowField[navLocation.NodeRef];
	}

	return FVector::Zero();
}


// Called when the game starts
void UFlowFieldComponent::BeginPlay()
{
	Super::BeginPlay();

	UNavigationSystemV1* navSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!navSys)
	{
		return;
	}

	navSys->OnNavigationGenerationFinishedDelegate.AddDynamic(this, &UFlowFieldComponent::CheckForInvalidNodeRefs);
	ARecastNavMesh* recastNavMesh = Cast<ARecastNavMesh>(navSys->GetDefaultNavDataInstance());
	if (!recastNavMesh)
	{
		return;
	}

	ActiveNavMesh = recastNavMesh;
}

inline void UFlowFieldComponent::GenerateFlowField(FVector targetLocation)
{
	GenerateFlowField(targetLocation, FieldRadius);
}

inline void UFlowFieldComponent::GenerateFlowField(FNavLocation targetNavLocation)
{
	GenerateFlowField(targetNavLocation, FieldRadius);
}

void UFlowFieldComponent::GenerateFlowField(FVector targetLocation, float radius)
{
	if (!IsValid(ActiveNavMesh))
	{
		return;
	}

	FNavLocation targetNavLocation;
	ActiveNavMesh->ProjectPoint(targetLocation, targetNavLocation, { 100,100,100 });
	GenerateFlowField(targetNavLocation, radius);
}

void UFlowFieldComponent::GenerateFlowField(FNavLocation targetNavLocation, float radius)
{
	if (!IsValid(ActiveNavMesh))
	{
		return;
	}

	FlowField.Empty();

	CurrentNodeRef = targetNavLocation.NodeRef;
	TArray<NavNodeRef> polys;
	TArray<NavNodeRef> parentPolys;
	TArray<float> costs;
	int polyCount;

	ActiveNavMesh->FindPolysAroundCircle(targetNavLocation.Location, CurrentNodeRef, radius, ActiveNavMesh->GetDefaultQueryFilter(), this, &polys, &parentPolys, &costs, &polyCount);

	for (int i = 0; i < polyCount; i++)
	{
		if (!parentPolys[i] || !polys[i] || FlowField.Contains(polys[i]))
		{
			continue;
		}

		FVector polyCentre;
		FVector parentPolyCentre;
		ActiveNavMesh->GetPolyCenter(polys[i], polyCentre);
		ActiveNavMesh->GetPolyCenter(parentPolys[i], parentPolyCentre);

		FVector direction = (parentPolyCentre - polyCentre).GetSafeNormal();
		FlowField.Add(polys[i], direction);

#if WITH_EDITOR
		if (DrawFlowField)
		{
			DrawDebugDirectionalArrow(GetWorld(), polyCentre, polyCentre + direction * 100, 100, FColor::Red, false, 30.f, 0, 20.f);
		}
#endif	
	}
}

void UFlowFieldComponent::CheckForInvalidNodeRefs(ANavigationData* navData)
{
	if (!IsValid(ActiveNavMesh) /*|| navData != ActiveNavMesh*/)
	{
		return;
	}

	FVector polyCenter;
	TArray<uint64> polyKeys;
	FlowField.GenerateKeyArray(polyKeys);

	for (uint64 poly : polyKeys)
	{
		//Their is no real way to check if a poly ref is valid or not, but if it's invalid then GetPolyCenter will fail and return false, so I just check for that
		if (!ActiveNavMesh->GetPolyCenter(poly, polyCenter))
		{
			GenerateFlowField(GetComponentLocation());
		}
	}
}


// Called every frame
void UFlowFieldComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(ActiveNavMesh))
	{
		return;
	}

	FNavLocation targetNavLocation;
	ActiveNavMesh->ProjectPoint(GetComponentLocation(), targetNavLocation, {100,100,100});

	if (targetNavLocation.NodeRef != CurrentNodeRef)
	{
		GenerateFlowField(targetNavLocation);
	}
}

