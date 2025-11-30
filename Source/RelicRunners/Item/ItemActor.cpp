/************************************************************************************
 * Author:   Tristan Anglin
 * Year:     2025
 *
 * Notice:
 *   This source code is the intellectual property of Tristan Anglin.
 *   It is provided solely for use within:
 *     - The Relic Runners personal project
 *     - The Almost A Dozen project
 *
 *   Any use, distribution, or modification outside of these projects
 *   is strictly prohibited without explicit written permission.
 *
 *   © 2025 Tristan Anglin. All rights reserved.
 ************************************************************************************/

#include "ItemActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LocalFogVolumeComponent.h"
#include "Components/WidgetComponent.h"
#include "ItemStats.h"
#include "ItemData.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Inventory/InventoryToolTip.h"
#include "Net/UnrealNetwork.h"

AItemActor::AItemActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    Fog = CreateDefaultSubobject<ULocalFogVolumeComponent>(TEXT("Fog"));
    Fog->SetupAttachment(RootComponent);

    TooltipWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("TooltipWidget"));
    TooltipWidgetComponent->SetupAttachment(RootComponent);
    TooltipWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
    TooltipWidgetComponent->SetDrawAtDesiredSize(true);
    TooltipWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 0));
    TooltipWidgetComponent->SetRelativeScale3D(FVector(0.1f));
    TooltipWidgetComponent->CastShadow = false;

    ItemCoverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemCoverMesh"));
    ItemCoverMesh->SetupAttachment(RootComponent);

    static ConstructorHelpers::FClassFinder<UUserWidget> TooltipWidgetBPClass(TEXT("/Game/Inventory/BP_InventoryToolTipWorld"));
    if (TooltipWidgetBPClass.Succeeded())
    {
        TooltipWidgetClass = TooltipWidgetBPClass.Class;
        TooltipWidgetComponent->SetWidgetClass(TooltipWidgetClass);
    }
}

void AItemActor::InitializeTooltipWidget()
{
    if (!TooltipWidgetComponent->GetUserWidgetObject())
    {
        TooltipWidgetComponent->SetWidgetClass(TooltipWidgetClass);
        TooltipWidgetComponent->InitWidget();
    }

    TooltipWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
    TooltipWidgetComponent->SetDrawAtDesiredSize(true);
    TooltipWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 0));
    TooltipWidgetComponent->SetRelativeScale3D(FVector(0.1f));
    TooltipWidgetComponent->SetPivot(FVector2D(0.5f, 1.f));
    TooltipWidgetComponent->CastShadow = false;
}


void AItemActor::BeginPlay()
{
    Super::BeginPlay();
}

void AItemActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FRotator Rotation = GetActorRotation();
    Rotation.Yaw += DeltaTime * 45.f;
    SetActorRotation(Rotation);

    if (TooltipWidgetComponent && GetWorld())
    {
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
}

void AItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AItemActor, ItemData);
}

void AItemActor::Initialize(const FItemData& InItemData)
{
    ItemData = InItemData;

    if (HasAuthority())
    {
        SetupVisuals();
    }
}

void AItemActor::OnRep_ItemData()
{
    SetupVisuals();
}

void AItemActor::SetupVisuals()
{
    if (!ItemData.ItemStaticMesh) return;

    if (StaticMeshComponent)
    {
        StaticMeshComponent->DestroyComponent();
        StaticMeshComponent = nullptr;
    }

    StaticMeshComponent = NewObject<UStaticMeshComponent>(this, TEXT("DroppedStaticMesh"));
    StaticMeshComponent->SetStaticMesh(ItemData.ItemStaticMesh);
    StaticMeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
    StaticMeshComponent->RegisterComponent();
    StaticMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -45.f));
    StaticMeshComponent->SetSimulatePhysics(false);
    StaticMeshComponent->SetEnableGravity(false);
    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    StaticMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    StaticMeshComponent->SetMobility(EComponentMobility::Stationary);
    StaticMeshComponent->CastShadow = false;
    
    UMaterialInterface* OutlineOverlay = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Shader/M_Outline"));
    
    if (OutlineOverlay)
    {
        StaticMeshComponent->SetOverlayMaterial(OutlineOverlay);
        StaticMeshComponent->SetForceDisableNanite(true);
        UE_LOG(LogTemp, Warning, TEXT("DropOutlineApplied"));
    }

    static const TMap<FString, FVector> ItemScaleMap = {
        { "Lower", FVector(1.50f) }, { "Upper", FVector(1.75f) },
        { "Mainhand", FVector(2.0f) }, 
        { "Helmet", FVector(3.0f) }, { "Arms", FVector(2.5f) }, 
        { "Necklace", FVector(3.25f) },
    };

    if (const FVector* Scale = ItemScaleMap.Find(ItemData.ItemType))
    {
        StaticMeshComponent->SetRelativeScale3D(*Scale);
    }

    // Setup fog
    const auto& RarityData = ItemStats::GetRarityDataMap()[ItemData.Rarity];

    Fog->SetRelativeLocation(FVector(0.f, 0.f, -45.f));
    Fog->SetRelativeScale3D(FVector(0.095f, 0.095f, 0.095f));
    Fog->FogPhaseG = 0.0f;
    Fog->HeightFogExtinction = 0.3f;
    Fog->HeightFogFalloff = 1;      
    Fog->FogAlbedo = RarityData.Color;             // tints the fog
    Fog->FogEmissive = RarityData.Color * 7.0f;    // strong inner glow
    Fog->MarkRenderStateDirty();

    // Tooltip setup
    InitializeTooltipWidget();

    if (UUserWidget* Widget = TooltipWidgetComponent->GetUserWidgetObject())
    {
        if (UInventoryToolTip* Tooltip = Cast<UInventoryToolTip>(Widget))
        {
            Tooltip->Setup(ItemData, RarityData.Color);
        }
    }
}

void AItemActor::ShowTooltip_Implementation(bool bShow)
{
    if (TooltipWidgetComponent)
    {
        TooltipWidgetComponent->SetVisibility(bShow);
    }
}

void AItemActor::Interact_Implementation(ARelicRunnersCharacter* Char)
{
    if (HasAuthority())
    {
        HandlePickup(Char);
    }
    else
    {
        Server_HandlePickup(Char);
    }
}

FItemData AItemActor::GetItemData_Implementation()
{
    return ItemData;
}

void AItemActor::Server_HandlePickup_Implementation(ARelicRunnersCharacter* Char)
{
    if (HasAuthority() && Char)
    {
        HandlePickup(Char);
    }
}

void AItemActor::HandlePickup(ARelicRunnersCharacter* Char)
{
    if (!Char) return;

    if (UInventoryComponent* InventoryComponent = Char->GetInventoryComponent())
    {
        UItemObject* Item = ItemStats::CreateItemFromData(ItemData, InventoryComponent);
        Item->IsNewItem = true;
        InventoryComponent->AddItem(Item);
        Destroy();
    }
}
