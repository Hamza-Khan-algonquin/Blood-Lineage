// Fill out your copyright notice in the Description page of Project Settings.


#include "HitFlashComponent.h"
#include <Net/UnrealNetwork.h>
#include "Curves/CurveFloat.h"

// Sets default values for this component's properties
UHitFlashComponent::UHitFlashComponent()
{
	SetIsReplicatedByDefault(true);
	HitFlashTimeline.SetLooping(false);
	PrimaryComponentTick.bCanEverTick = true;
}

void UHitFlashComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHitFlashComponent, DynamicMaterials);
	DOREPLIFETIME(UHitFlashComponent, HitFlashColor);
}

//End olay runs across all game instances so the hit flash effect is properly ended 
void UHitFlashComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	EndHitFlash();
	Super::EndPlay(EndPlayReason);
}

//Begin play runs across all game instances so the hit flash effect is ready for all clients and the server
void UHitFlashComponent::BeginPlay()
{
	Super::BeginPlay();

	
	UpdateDynamicMaterialColors();

	//Set up the hit flash timeline. This timeline will drive the hit flash effect by using the HitFlashCurve to determine the strength of the hit flash at any given time.
	if (HitFlashCurve.IsValid() || HitFlashCurve.LoadSynchronous())
	{
		HitFlashTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);

		FOnTimelineFloat inProgressFunc;
		inProgressFunc.BindUFunction(this, FName("UpdateHitFlash"));
		HitFlashTimeline.AddInterpFloat(HitFlashCurve.Get(), inProgressFunc);

		FOnTimelineEvent endFunc;
		endFunc.BindUFunction(this, FName("EndHitFlash"));
		HitFlashTimeline.SetTimelineFinishedFunc(endFunc);

		HitFlashTimeline.SetLooping(false);
		HitFlashTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
	}
#if WITH_EDITOR || UE_BUILD_TEST
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid HitFlashCurve on %s. Hit flash effect will not work."), *GetOwner()->GetName());
	}
#endif

}


// Called every frame
void UHitFlashComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HitFlashTimeline.TickTimeline(DeltaTime);
}

void UHitFlashComponent::StartHitFlash()
{
	//ShouldHitFlash = true;
	HitFlashTimeline.PlayFromStart();
	SetComponentTickEnabled(true);
}

void UHitFlashComponent::Multicast_StartHitFlash_Implementation()
{
	StartHitFlash();
}

void UHitFlashComponent::EndHitFlash()
{
	//ShouldHitFlash = false;
	HitFlashTimeline.Stop();
	UpdateHitFlash(0);

	SetComponentTickEnabled(false);
}

void UHitFlashComponent::Multicast_EndHitFlash_Implementation()
{
	EndHitFlash();
}

void UHitFlashComponent::InitializeNewDynamicMaterials(UMeshComponent* newComponent)
{
	if (!newComponent)
	{
#if WITH_EDITOR || UE_BUILD_TEST
		UE_LOG(LogTemp, Error, TEXT("Invalid component passed to InitializeNewDynamicMaterials"));
#endif
		return;
	}

	for (int i = 0; i < newComponent->GetNumMaterials(); i++)
	{
		UMaterialInstanceDynamic* material = Cast<UMaterialInstanceDynamic>(newComponent->GetMaterial(i));

		if (material && !DynamicMaterials.Contains(material))
		{
			DynamicMaterials.Add(material);
		}
		else if (!material)
		{
			DynamicMaterials.Add(newComponent->CreateAndSetMaterialInstanceDynamic(i));
		}
	}
}

void UHitFlashComponent::RemoveNewDynamicMaterials(UMeshComponent* removedComponent)
{
	if (!removedComponent)
	{
#if WITH_EDITOR || UE_BUILD_TEST
		UE_LOG(LogTemp, Error, TEXT("Invalid component passed to RemoveNewDynamicMaterials"));
#endif
		return;
	}

	for (int i = 0; i < removedComponent->GetNumMaterials(); i++)
	{
		UMaterialInstanceDynamic* material = Cast<UMaterialInstanceDynamic>(removedComponent->GetMaterial(i));

		if (material)
		{
			DynamicMaterials.Remove(material);
		}
	}
}

void UHitFlashComponent::SetHitFlashColor(const FColor newColor)
{
	HitFlashColor = newColor;
	UpdateDynamicMaterialColors();
}

void UHitFlashComponent::UpdateHitFlash(const float strength)
{
	for (UMaterialInstanceDynamic* element : DynamicMaterials)
	{
		if (element)
		{
			element->SetScalarParameterValue("HitFlashStrength", strength);
		}
	}
}

void UHitFlashComponent::UpdateDynamicMaterialColors_Implementation()
{
	for (UMaterialInstanceDynamic* element : DynamicMaterials)
	{
		if (element)
		{
			element->SetVectorParameterValue("HitFlashColor", HitFlashColor);
		}
	}
}
