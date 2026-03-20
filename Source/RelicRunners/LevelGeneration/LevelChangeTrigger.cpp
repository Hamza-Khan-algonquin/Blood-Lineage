// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelChangeTrigger.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "../PlayerController/RelicRunnersPlayerController.h"
#include "RelicRunners/RelicRunnersGameState.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ALevelChangeTrigger::ALevelChangeTrigger():
	Origin(nullptr),
	TriggerBox(nullptr),
	TargetLevel("")
{
	PrimaryActorTick.bCanEverTick = false;

	Origin = CreateDefaultSubobject<USceneComponent>("Origin");
	RootComponent = Origin;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>("TriggerCollision");
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionProfileName("OverlapAll");
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ALevelChangeTrigger::OnTriggerOverlap);
	TriggerBox->SetupAttachment(Origin);
	TriggerBox->SetBoxExtent(FVector(50.0f));
	TriggerBox->SetRelativeLocation(FVector(0.0f, 0.0f, TriggerBox->GetUnscaledBoxExtent().Z));

	LevelTargetTextRender = CreateDefaultSubobject<UTextRenderComponent>("LevelTextRender");
	LevelTargetTextRender->SetupAttachment(Origin);

	bReplicates = true;
	bAlwaysRelevant = true;
}

void ALevelChangeTrigger::OnConstruction(const FTransform& transform)
{
	TriggerBox->SetRelativeLocation(FVector(0.0f, 0.0f, TriggerBox->GetUnscaledBoxExtent().Z));

	LevelTargetTextRender->SetText(FText::FromString(TargetLevel));
}

// Called when the game starts or when spawned
void ALevelChangeTrigger::BeginPlay()
{
	Super::BeginPlay();

	LevelTargetTextRender->SetText(FText::FromString(TargetLevel));
}

void ALevelChangeTrigger::OnTriggerOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->ActorHasTag("Player")) return;

	if (PortalEnterSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PortalEnterSound, GetActorLocation());
	}

	if (HasAuthority())
	{
		//Server_ChangeLevel();
		GetWorldTimerManager().SetTimer(LevelTravelHandle, this, &ALevelChangeTrigger::ChangeLevel, 0.2f, false);
	}
}

void ALevelChangeTrigger::Server_ChangeLevel_Implementation()
{
	ChangeLevel();
}

void ALevelChangeTrigger::Server_SetTargetLevel_Implementation(FName newTargetLevel)
{
	TargetLevel = newTargetLevel.ToString();

	LevelTargetTextRender->SetText(FText::FromString(TargetLevel));
}

void ALevelChangeTrigger::ChangeLevel()
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(World->GetFirstPlayerController());
	if (!RPC) return;

	RPC->CleanUI();

	FString TravelURL = TargetLevel + TEXT("?listen");
	World->ServerTravel(TravelURL, true);
}

void ALevelChangeTrigger::Server_Activate_Implementation()
{
	IsActive = true;
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Level Trigger has been activated")));
}

void ALevelChangeTrigger::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALevelChangeTrigger, TargetLevel);
	DOREPLIFETIME(ALevelChangeTrigger, IsActive);
}
