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

#include "PlayerPreview.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "RelicRunners/Inventory/Inventory.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/SpringArmComponent.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include <Kismet/GameplayStatics.h>

APlayerPreview::APlayerPreview()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootComponent);

	BodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);

	SceneCaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCaptureComponent2D->SetupAttachment(RootComponent);
	SceneCaptureComponent2D->bCaptureEveryFrame = true;
	SceneCaptureComponent2D->bCaptureOnMovement = false;

	UpperMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("UpperMesh"));
	UpperMesh->SetupAttachment(BodyMesh);
	UpperMesh->SetLeaderPoseComponent(BodyMesh);

	LowerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LowerMesh"));
	LowerMesh->SetupAttachment(BodyMesh);
	LowerMesh->SetLeaderPoseComponent(BodyMesh);

	ArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsMesh"));
	ArmsMesh->SetupAttachment(BodyMesh);
	ArmsMesh->SetLeaderPoseComponent(BodyMesh);

	HelmetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HelmetMesh"));
	HelmetMesh->SetupAttachment(BodyMesh);
	HelmetMesh->SetLeaderPoseComponent(BodyMesh);

	NecklaceMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("NecklaceMesh"));
	NecklaceMesh->SetupAttachment(BodyMesh);
	NecklaceMesh->SetLeaderPoseComponent(BodyMesh);

	MainhandItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainhandMesh"));
	MainhandItemMesh->SetupAttachment(BodyMesh, TEXT("MainhandSocket"));

	OffhandItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OffhandMesh"));
	OffhandItemMesh->SetupAttachment(BodyMesh, TEXT("OffhandSocket"));

	bReplicates = false;
	bAlwaysRelevant = false;
	SetReplicateMovement(false);
	SetActorHiddenInGame(false);
}

void APlayerPreview::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerPreview::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerPreview::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void APlayerPreview::ClearAllPreviewMeshes()
{
	//Skeletal meshes (armor, accessories)
	UpperMesh->SetSkeletalMesh(nullptr);
	LowerMesh->SetSkeletalMesh(nullptr);
	ArmsMesh->SetSkeletalMesh(nullptr);
	HelmetMesh->SetSkeletalMesh(nullptr);
	NecklaceMesh->SetSkeletalMesh(nullptr);

	//Static meshes (weapons)
	MainhandItemMesh->SetStaticMesh(nullptr);
	OffhandItemMesh->SetStaticMesh(nullptr);
}

UTextureRenderTarget2D* APlayerPreview::CreateRenderTarget()
{
	//Create and assign a unique render target
	UTextureRenderTarget2D* NewRenderTarget = NewObject<UTextureRenderTarget2D>();
	NewRenderTarget->RenderTargetFormat = RTF_RGBA16f;
	NewRenderTarget->InitAutoFormat(300, 500);
	NewRenderTarget->ClearColor = FLinearColor(0, 0, 0, 0); 
	NewRenderTarget->bAutoGenerateMips = false;
	NewRenderTarget->UpdateResourceImmediate(true);

	SceneCaptureComponent2D->TextureTarget = NewRenderTarget;
	SceneCaptureComponent2D->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	SceneCaptureComponent2D->ShowFlags.SetAtmosphere(false);
	SceneCaptureComponent2D->ShowFlags.SetFog(false);
	SceneCaptureComponent2D->ShowFlags.SetDynamicShadows(false);

	UMaterialInterface* PostMat = LoadObject<UMaterialInterface>(
		nullptr, TEXT("/Game/ThirdPerson/Blueprints/PlayerPreview/M_PlayerPreviewPostProcess.M_PlayerPreviewPostProcess"));

	if (PostMat)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SceneCapture] Successfully set post process material."));
		SceneCaptureComponent2D->PostProcessSettings.AddBlendable(PostMat, 1.0f);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[SceneCapture] Failed to set post process material."));
	}

	RenderTarget = NewRenderTarget;
	return RenderTarget;
}

USkeletalMeshComponent* APlayerPreview::GetSkeletalMeshComponentByItemType(const FString& ItemType)
{
	if (ItemType == "Upper") return UpperMesh;
	if (ItemType == "Lower") return LowerMesh;
	if (ItemType == "Arms") return ArmsMesh;
	if (ItemType == "Helmet") return HelmetMesh;
	if (ItemType == "Necklace") return NecklaceMesh;
	return nullptr;
}

UStaticMeshComponent* APlayerPreview::GetStaticMeshComponentByItemType(const FString& ItemType)
{
	if (ItemType == "Greatsword") return MainhandItemMesh;
	if (ItemType == "Sword") return MainhandItemMesh;
	if (ItemType == "Staff") return MainhandItemMesh;
	if (ItemType == "Bow") return MainhandItemMesh;
	return nullptr;
}

void APlayerPreview::SetPreviewMeshByItemType(const FString& ItemType, UObject* MeshAsset)
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