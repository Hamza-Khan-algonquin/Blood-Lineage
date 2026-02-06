// Fill out your copyright notice in the Description page of Project Settings.


#include "NewSurvivalZone.h"
#include "RelicRunners/RelicRunnersGameState.h"
#include "Components/SphereComponent.h"
#include <Net/UnrealNetwork.h>

// Sets default values
ANewSurvivalZone::ANewSurvivalZone() :
	TriggerZone(nullptr),
	SurvivalTimelineComp(nullptr),
	SurvivalTimelineFloatCurve(nullptr),
	Duration(0.0f),
	Remaining(0.0f)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TriggerZone = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerZone"));
	TriggerZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerZone->SetCollisionProfileName("OverlapAll");
	TriggerZone->SetGenerateOverlapEvents(true);
	TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &ANewSurvivalZone::OnTriggerOverlap);
	TriggerZone->OnComponentEndOverlap.AddDynamic(this, &ANewSurvivalZone::OnOverlapEnd);
	TriggerZone->SetupAttachment(Origin);

	SurvivalTimelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("SurvivalTimelineComponent"));
}

// Called when the game starts or when spawned
void ANewSurvivalZone::BeginPlay()
{
	Super::BeginPlay();

	if (SurvivalTimelineComp && SurvivalTimelineFloatCurve)
	{
		FOnTimelineFloat UpdateFunctionFloat;
		UpdateFunctionFloat.BindUFunction(this, FName("UpdateTimelineComp"));
		SurvivalTimelineComp->AddInterpFloat(SurvivalTimelineFloatCurve, UpdateFunctionFloat);

		FOnTimelineEvent TimelineFinishedEvent;
		TimelineFinishedEvent.BindUFunction(this, FName("OnTimelineEnd"));
		SurvivalTimelineComp->SetTimelineFinishedFunc(TimelineFinishedEvent);

		SurvivalTimelineComp->SetTimelineLength(1.0f);
		SurvivalTimelineComp->SetPlayRate(1.0f / Duration);
		SurvivalTimelineComp->SetLooping(false);
	}
}

// Called every frame
void ANewSurvivalZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ANewSurvivalZone::OnTriggerOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() && OtherActor->ActorHasTag("Player"))
	{
		Multicast_HandleOverlapEnter(OtherActor, true);
	}
}

void ANewSurvivalZone::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (HasAuthority() && Remaining > 0.0f)
	{
		Multicast_HandleOverlapEnter(OtherActor, false);
	}
}

void ANewSurvivalZone::UpdateTimelineComp(float Output)
{
	Remaining = Output;
	GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Orange, FString::Printf(TEXT("Remaining time: %f"), Remaining * Duration));
}

void ANewSurvivalZone::OnTimelineEnd()
{
	if (Remaining > 0.0f)
		return;

	TriggerZone->SetGenerateOverlapEvents(false);

	GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Green, FString::Printf(TEXT("Survived the zone.")));
	OnObjectiveActivated.Broadcast();
}

void ANewSurvivalZone::Multicast_HandleOverlapEnter_Implementation(AActor* OtherActor, bool OnOverlapBegin)
{
	if (OnOverlapBegin)
	{
		CollidingActors.AddUnique(OtherActor);
		SurvivalTimelineComp->SetPlayRate(1.0f / Duration);
		SurvivalTimelineComp->Play();
	}
	else
	{
		CollidingActors.Remove(OtherActor);

		if (CollidingActors.IsEmpty())
		{
			SurvivalTimelineComp->SetPlayRate(1.0f / Duration / 3.0f);
			SurvivalTimelineComp->Reverse();
		}
	}
}

void ANewSurvivalZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANewSurvivalZone, Duration);
	DOREPLIFETIME(ANewSurvivalZone, CollidingActors);
}