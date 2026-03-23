// Fill out your copyright notice in the Description page of Project Settings.


#include "ForgeActor.h"
#include <RelicRunners/PlayerController/RelicRunnersPlayerController.h>
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "RelicRunners/Item/ItemStats.h"
#include "RelicRunners/Item/ItemData.h"
#include "RelicRunners/Inventory/Inventory.h"
#include "Components/CanvasPanel.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include <Kismet/GameplayStatics.h>

AForgeActor::AForgeActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		Capsule->SetSimulatePhysics(false);
		Capsule->SetEnableGravity(false);
	}

	if (GetMesh())
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	}
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

	bAlwaysRelevant = true;
}

void AForgeActor::InitializeTooltipWidget()
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

void AForgeActor::ShowTooltip_Implementation(bool bShow)
{
	if (TooltipWidgetComponent)
	{
		TooltipWidgetComponent->SetVisibility(bShow);
	}
}

void AForgeActor::BeginPlay()
{
	Super::BeginPlay();

	InitializeTooltipWidget();
}

// Called every frame
void AForgeActor::Tick(float DeltaTime)
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

FItemData AForgeActor::GetItemData_Implementation()
{
	return FItemData();
}

void AForgeActor::Interact_Implementation(ARelicRunnersCharacter* Char)
{
	if (!Char) return;

	ARelicRunnersPlayerController* PC = Char->GetController<ARelicRunnersPlayerController>();
	if (!PC) return;

	LinkedInventoryWidget = PC->GetInventory();

	if (LinkedInventoryWidget)
	{
		if (!LinkedInventoryWidget->IsVisible())
		{
			PC->InventoryUI();
		}
		if (!LinkedInventoryWidget->ForgeCanvas->IsVisible())
		{
			LinkedInventoryWidget->ToggleForgeUI(true);
			LinkedInventoryWidget->CurrentVendor = this;
		}
		else
		{
			LinkedInventoryWidget->ToggleForgeUI(false);
		}

	}
}

