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

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerPreview.generated.h"

UCLASS()
class RELICRUNNERS_API APlayerPreview : public AActor
{
	GENERATED_BODY()

public:
	APlayerPreview();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UTextureRenderTarget2D* RenderTarget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	TObjectPtr<UMaterialInterface> PostProcessMaterial;

	class UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneCaptureComponent2D* SceneCaptureComponent2D;

	class ARelicRunnersCharacter* OwningCharacter;

	UPROPERTY(VisibleAnywhere)
	TSubclassOf<UAnimInstance> HealerAnim;

	UPROPERTY(VisibleAnywhere)
	TSubclassOf<UAnimInstance> MeleeAnim;

	UPROPERTY(VisibleAnywhere)
	TSubclassOf<UAnimInstance> RangedAnim;

	UPROPERTY(VisibleAnywhere)
	TSubclassOf<UAnimInstance> TankAnim;

	UPROPERTY(EditAnywhere)
	UStaticMesh* ShieldAsset;

	UPROPERTY(EditAnywhere)
	USkeletalMesh* AresModel;

	UPROPERTY(EditAnywhere)
	USkeletalMesh* NemesisModel;

	UPROPERTY(EditAnywhere)
	USkeletalMesh* ArtemisModel;

	UPROPERTY(EditAnywhere)
	USkeletalMesh* AphroditeModel;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* BodyMesh;

	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* UpperMesh;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* HelmetMesh;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* LowerMesh;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* ArmsMesh;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* NecklaceMesh;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MainhandItemMesh;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* OffhandItemMesh;

	class UTextureRenderTarget2D* CreateRenderTarget();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void ClearAllPreviewMeshes();
	USkeletalMeshComponent* GetSkeletalMeshComponentByItemType(const FString& ItemType);
	UStaticMeshComponent* GetStaticMeshComponentByItemType(const FString& ItemType);
	void SetPreviewMeshByItemType(const FString& ItemType, UObject* MeshAsset);
};
