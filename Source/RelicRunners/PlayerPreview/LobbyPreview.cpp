// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPreview.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include <RelicRunners/PlayerState/RelicRunnersPlayerState.h>
#include <Net/UnrealNetwork.h>
#include <RelicRunners/PlayerHUD/LobbyHUDWorld.h>

ALobbyPreview::ALobbyPreview()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;

    static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBP(
        TEXT("/Game/Characters/Mannequins/Animations/ABP_Quinn.ABP_Quinn_C")
    );

    if (AnimBP.Succeeded())
    {
        MeshComp->SetAnimInstanceClass(AnimBP.Class);
    }

    MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    MeshComp->SetIsReplicated(true);

    UpperMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("UpperMesh"));
    UpperMesh->SetupAttachment(MeshComp);
    UpperMesh->SetLeaderPoseComponent(MeshComp);

    LowerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LowerMesh"));
    LowerMesh->SetupAttachment(MeshComp);
    LowerMesh->SetLeaderPoseComponent(MeshComp);

    ArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsMesh"));
    ArmsMesh->SetupAttachment(MeshComp);
    ArmsMesh->SetLeaderPoseComponent(MeshComp);

    HelmetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HelmetMesh"));
    HelmetMesh->SetupAttachment(MeshComp);
    HelmetMesh->SetLeaderPoseComponent(MeshComp);

    NecklaceMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("NecklaceMesh"));
    NecklaceMesh->SetupAttachment(MeshComp);
    NecklaceMesh->SetLeaderPoseComponent(MeshComp);

    MainhandItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainhandMesh"));
    MainhandItemMesh->SetupAttachment(MeshComp, TEXT("MainhandSocket"));

    OffhandItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OffhandMesh"));
    OffhandItemMesh->SetupAttachment(MeshComp, TEXT("OffhandSocket"));

    LobbyHUDWorld = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
    LobbyHUDWorld->SetupAttachment(MeshComp);
    LobbyHUDWorld->SetWorldScale3D(FVector(0.1, 0.1, 0.1));
    LobbyHUDWorld->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
    LobbyHUDWorld->SetWidgetSpace(EWidgetSpace::World);
    LobbyHUDWorld->SetDrawAtDesiredSize(true);
    LobbyHUDWorld->SetVisibility(true);
    LobbyHUDWorld->CastShadow = false;
}

void ALobbyPreview::BeginPlay()
{
	Super::BeginPlay();

    if (LobbyHUDWorld)
    {
        if (UUserWidget* Widget = LobbyHUDWorld->GetUserWidgetObject())
        {
            if (ULobbyHUDWorld* LobbyWidget = Cast<ULobbyHUDWorld>(Widget))
            {
                LobbyWidget->OwningPreview = this;
            }
        }
    }
	
}

void ALobbyPreview::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALobbyPreview::SetupFromPlayerState(ARelicRunnersPlayerState* PS)
{
    LinkedPlayerState = PS;

    if (HasAuthority())
    {
        OnRep_LinkedPlayerState();
    }
}

void ALobbyPreview::OnRep_LinkedPlayerState()
{
    if (LinkedPlayerState)
    {
        LinkedPlayerState->OnSelectedClassChanged.RemoveAll(this);
        LinkedPlayerState->OnSelectedClassChanged.AddUObject(this, &ALobbyPreview::OnPlayerClassChanged);
        OnPlayerClassChanged(LinkedPlayerState->SelectedClass);
        OnPlayerStateReady.Broadcast();
    }
}

void ALobbyPreview::OnPlayerClassChanged(FName NewClass)
{
    if (!LinkedPlayerState) return;

    UE_LOG(LogTemp, Log, TEXT("[LobbyPreview] %s -> %s"),*GetNameSafe(LinkedPlayerState), *NewClass.ToString());

    FString ItemType;
    if (NewClass == "Ares")             ItemType = "Greatsword";
    else if (NewClass == "Nemesis")     ItemType = "Sword";
    else if (NewClass == "Aphrodite")   ItemType = "Staff";
    else if (NewClass == "Artemis")     ItemType = "Bow";
    if (ItemType.IsEmpty()) return;

    FString MeshPath = GetMeshPathByItemType(ItemType);

    UObject* LoadedMesh = nullptr;
    if (ItemType == "Staff" || ItemType == "Sword" || ItemType == "Greatsword" || ItemType == "Bow" || ItemType == "Offhand")
    {
        LoadedMesh = StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *MeshPath);
        SetPreviewMeshByItemType(ItemType, Cast<UStaticMesh>(LoadedMesh));
    }
    else
    {
        LoadedMesh = StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, *MeshPath);
        SetPreviewMeshByItemType(ItemType, Cast<USkeletalMesh>(LoadedMesh));
    }
}

void ALobbyPreview::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALobbyPreview, LinkedPlayerState);
}

UStaticMeshComponent* ALobbyPreview::GetStaticMeshComponentByItemType(const FString& ItemType)
{
    if (ItemType == "Greatsword") return MainhandItemMesh;
    if (ItemType == "Sword") return MainhandItemMesh;
    if (ItemType == "Staff") return MainhandItemMesh;
    if (ItemType == "Bow") return MainhandItemMesh;
    return nullptr;
}

FString ALobbyPreview::GetMeshPathByItemType(const FString& ItemType)
{
    return FString::Printf(TEXT("/Game/ThirdPerson/Meshes/Items/%s/%s.%s"),
        *ItemType, *ItemType, *ItemType);
}

void ALobbyPreview::SetPreviewMeshByItemType(const FString& ItemType, UObject* MeshAsset)
{
    if (ItemType == "Upper")
    {
        USkeletalMesh* Mesh = Cast<USkeletalMesh>(MeshAsset);
        if (UpperMesh && Mesh) UpperMesh->SetSkeletalMesh(Mesh);
    }
    else if (ItemType == "Lower")
    {
        USkeletalMesh* Mesh = Cast<USkeletalMesh>(MeshAsset);
        if (LowerMesh && Mesh) LowerMesh->SetSkeletalMesh(Mesh);
    }
    else if (ItemType == "Arms")
    {
        USkeletalMesh* Mesh = Cast<USkeletalMesh>(MeshAsset);
        if (ArmsMesh && Mesh) ArmsMesh->SetSkeletalMesh(Mesh);
    }
    else if (ItemType == "Helmet")
    {
        USkeletalMesh* Mesh = Cast<USkeletalMesh>(MeshAsset);
        if (HelmetMesh && Mesh) HelmetMesh->SetSkeletalMesh(Mesh);
    }
    else if (ItemType == "Necklace")
    {
        USkeletalMesh* Mesh = Cast<USkeletalMesh>(MeshAsset);
        if (NecklaceMesh && Mesh) NecklaceMesh->SetSkeletalMesh(Mesh);
    }
    else if (ItemType == "Sword" || ItemType == "Greatsword" || ItemType == "Staff" || ItemType == "Bow")
    {
        UStaticMesh* Mesh = Cast<UStaticMesh>(MeshAsset);
        if (MainhandItemMesh && Mesh) MainhandItemMesh->SetStaticMesh(Mesh);
    }
    else if (ItemType == "Offhand")
    {
        UStaticMesh* Mesh = Cast<UStaticMesh>(MeshAsset);
        if (OffhandItemMesh && Mesh) OffhandItemMesh->SetStaticMesh(Mesh);
    }
}