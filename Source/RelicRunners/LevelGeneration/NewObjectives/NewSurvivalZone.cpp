// Fill out your copyright notice in the Description page of Project Settings.


#include "NewSurvivalZone.h"
#include "RelicRunners/RelicRunnersGameState.h"
#include "Components/SphereComponent.h"
#include <Net/UnrealNetwork.h>
#include <RelicRunners/PlayerController/RelicRunnersPlayerController.h>

// Sets default values
ANewSurvivalZone::ANewSurvivalZone() :
	TriggerZone(nullptr),
	SurvivalTimelineComp(nullptr),
	SurvivalTimelineFloatCurve(nullptr),
	Duration(0.0f),
	Remaining(0.0f),
	CompletionPercentage(-1.0f)
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

	PandoraBoxMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Pandora's box");
	PandoraBoxMesh->SetupAttachment(Origin);

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

		Remaining = Duration;
		CompletionPercentage = 0.0f;
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
	if (Duration <= 0.0f) return;

	float Alpha = FMath::Clamp(Output, 0.0f, 1.0f);

	Remaining = Alpha * Duration;

	float NewPercent = Alpha;

	if (!FMath::IsNearlyEqual(NewPercent, CompletionPercentage, 0.001f))
	{
		CompletionPercentage = NewPercent;

		int TotalSecondsLeft = FMath::CeilToInt(Remaining);
		int Seconds = TotalSecondsLeft % 60;

		FString TimerText = FString::Printf(TEXT("%d"), Seconds);

		UpdateObjectiveToAllControllers(CompletionPercentage, TimerText);
	}
}
void ANewSurvivalZone::UpdateObjectiveToAllControllers(float NewPercent, FString TimerText)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ARelicRunnersPlayerController* TargetPC = Cast<ARelicRunnersPlayerController>(It->Get());
		if (TargetPC)
		{
			TargetPC->UpdatePandoraObjective(NewPercent, TimerText);
		}
	}
}

void ANewSurvivalZone::OnTimelineEnd()
{
	if (Remaining > 0.0f)
		return;

	TriggerZone->SetGenerateOverlapEvents(false);

	BeaconMesh->SetVisibility(false);

	OnObjectiveActivated.Broadcast();

	if (PandoraBoxMesh && ActivateAnimation)
	{
		PandoraBoxMesh->PlayAnimation(ActivateAnimation, false);
	}

}

void ANewSurvivalZone::Multicast_HandleOverlapEnter_Implementation(AActor* OtherActor, bool OnOverlapBegin)
{
	if (Duration <= 0.0f) return;

	float NormalRate = 1.0f / Duration;

	if (OnOverlapBegin)
	{
		CollidingActors.AddUnique(OtherActor);
		SurvivalTimelineComp->SetPlayRate(NormalRate);
		SurvivalTimelineComp->Play();
	}
	else
	{
		CollidingActors.Remove(OtherActor);
		if (CollidingActors.IsEmpty())
		{
			SurvivalTimelineComp->SetPlayRate(NormalRate / 3.0f);
			SurvivalTimelineComp->Reverse();
		}
	}
}

void ANewSurvivalZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANewSurvivalZone, Duration);
	DOREPLIFETIME(ANewSurvivalZone, CompletionPercentage);
	DOREPLIFETIME(ANewSurvivalZone, CollidingActors);
}