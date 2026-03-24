// Fill out your copyright notice in the Description page of Project Settings.


#include "NewCapturableFlag.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "RelicRunners/RelicRunnersGameState.h"
#include "RelicRunners/Game/RelicRunnersGameInstance.h"
#include "Components/StaticMeshComponent.h"
#include <Net/UnrealNetwork.h>
#include "Components/WidgetComponent.h"
#include <Kismet/GameplayStatics.h>

ANewCapturableFlag::ANewCapturableFlag()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>("TriggerCollision");
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionProfileName("BlockAll");
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetupAttachment(Origin);

	TooltipWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("TooltipWidget"));
	TooltipWidgetComponent->SetupAttachment(RootComponent);
	TooltipWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	TooltipWidgetComponent->SetDrawAtDesiredSize(true);
	TooltipWidgetComponent->SetUsingAbsoluteRotation(true);
	TooltipWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 0));
	TooltipWidgetComponent->SetRelativeScale3D(FVector(0.3f));
	TooltipWidgetComponent->SetPivot(FVector2D(0.5f, 1.f));
	TooltipWidgetComponent->CastShadow = false;

	static ConstructorHelpers::FClassFinder<UUserWidget> TooltipWidgetBPClass(TEXT("/Game/Inventory/BP_InventoryToolTipWorld"));
	if (TooltipWidgetBPClass.Succeeded())
	{
		TooltipWidgetClass = TooltipWidgetBPClass.Class;
		TooltipWidgetComponent->SetWidgetClass(TooltipWidgetClass);
	}

	bReplicates = true;
	bAlwaysRelevant = true;
}

void ANewCapturableFlag::InitializeTooltipWidget()
{
	if (!TooltipWidgetComponent->GetUserWidgetObject())
	{
		TooltipWidgetComponent->SetWidgetClass(TooltipWidgetClass);
		TooltipWidgetComponent->InitWidget();
	}

	TooltipWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	TooltipWidgetComponent->SetDrawAtDesiredSize(true);
	TooltipWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 200));
	TooltipWidgetComponent->SetRelativeScale3D(FVector(0.3f));
	TooltipWidgetComponent->SetPivot(FVector2D(0.5f, 1.f));
	TooltipWidgetComponent->CastShadow = false;
	TooltipWidgetComponent->SetComponentTickEnabled(true);

	if (!TooltipWidgetComponent->IsRegistered())
	{
		TooltipWidgetComponent->RegisterComponent();
	}
}

void ANewCapturableFlag::ShowTooltip_Implementation(bool bShow)
{
	if (TooltipWidgetComponent)
	{
		TooltipWidgetComponent->SetVisibility(bShow);
	}
}

void ANewCapturableFlag::BeginPlay()
{
	Super::BeginPlay();

	InitializeTooltipWidget();
}

void ANewCapturableFlag::Interact_Implementation(ARelicRunnersCharacter* Char)
{
}

void ANewCapturableFlag::Server_HandleInteracted_Implementation(ARelicRunnersCharacter* Char)
{
	if (HasAuthority() && Char)
	{
		HandleInteracted(Char);
	}
}

void ANewCapturableFlag::HandleInteracted(ARelicRunnersCharacter* Char)
{
	if (!Char || isActive) return;

	isActive = true;

	UpdateVisuals();

	OnObjectiveActivated.Broadcast();
}

void ANewCapturableFlag::UpdateVisuals()
{
	TextComponent->SetText(FText::FromString("Complete!"));
	BeaconMesh->SetVisibility(false);
	OnObjectiveActivated.Broadcast();
	PlayFlagAnimation();

	if (TooltipWidgetComponent)
	{
		TooltipWidgetComponent->SetHiddenInGame(true);
		TooltipWidgetComponent->SetVisibility(false);
	}
}

void ANewCapturableFlag::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!TooltipWidgetComponent) return;

	APlayerCameraManager* CamManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (CamManager)
	{
		FVector CameraLocation = CamManager->GetCameraLocation();
		FVector WidgetLocation = TooltipWidgetComponent->GetComponentLocation();

		FRotator LookAtRotation = (CameraLocation - WidgetLocation).Rotation();

		LookAtRotation.Pitch = 0.f;
		LookAtRotation.Roll = 0.f;

		TooltipWidgetComponent->SetWorldRotation(LookAtRotation);
	}
}
void ANewCapturableFlag::OnActiveChange()
{
	UpdateVisuals();
}

void ANewCapturableFlag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANewCapturableFlag, isActive);
}