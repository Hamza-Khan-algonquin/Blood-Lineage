// Fill out your copyright notice in the Description page of Project Settings.


#include "ShockwaveRing.h"
#include <Kismet/GameplayStatics.h>
#include "RelicRunners/RelicRunnersCharacter.h"
#include "Net/UnrealNetwork.h"

AShockwaveRing::AShockwaveRing()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    bReplicates = true;
    SetReplicateMovement(true);
}

void AShockwaveRing::InitializeRing(int Diff, float Speed)
{
    MoveSpeed = Speed;
    Difficulty = Diff;
    int TotalSteps = 12;
    float SliceWidth = 30.0f;

    float GapChance = 0.8f - (Difficulty * 0.08f);
    bool bAtLeastOneGap = false;

    for (int i = 0; i < TotalSteps; i++)
    {
        if (FMath::FRand() < GapChance || (i == TotalSteps - 1 && !bAtLeastOneGap))
        {
            bAtLeastOneGap = true;
            continue;
        }

        UStaticMeshComponent* Slice = NewObject<UStaticMeshComponent>(this);
        if (Slice)
        {
            Slice->RegisterComponent();
            Slice->SetStaticMesh(SliceMesh);
            UMaterialInstanceDynamic* DynMat = Slice->CreateDynamicMaterialInstance(0);
            if (DynMat)
            {
                SliceMaterials.Add(DynMat);
            }
            Slice->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
            Slice->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            Slice->SetCollisionResponseToAllChannels(ECR_Overlap);
            Slice->CanCharacterStepUpOn = ECB_No;
            Slice->SetIsReplicated(true);
            float Degrees = i * SliceWidth;
            float Radians = FMath::DegreesToRadians(Degrees);

            Slice->SetRelativeLocation(FVector(0));
            Slice->SetRelativeRotation(FRotator(0.f, Degrees - 90.f, 0.f));
            Slice->SetRelativeScale3D(FVector(5.0f));
            Slice->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
            Slice->OnComponentBeginOverlap.AddDynamic(this, &AShockwaveRing::OnSliceOverlap);
        }
    }
}

void AShockwaveRing::OnSliceOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == GetOwner() || OtherActor == this)
    {
        return;
    }

    if (AlreadyHitActors.Contains(OtherActor))
    {
        return;
    }

    if (OtherActor->IsA(ARelicRunnersCharacter::StaticClass()))
    {
        AlreadyHitActors.Add(OtherActor);

        //Base 12.5 with Multiplier 2.0 hits 400 at Difficulty 5
        //12.5 * 2^1 = 25
        //12.5 * 2^5 = 400
        float BaseDamage = 15.0f;
        float ExponentialDamage = BaseDamage * FMath::Pow(2.0f, Difficulty);

        UGameplayStatics::ApplyDamage(
            OtherActor,
            ExponentialDamage,
            GetInstigatorController(),
            this,
            UDamageType::StaticClass()
        );
    }
}

void AShockwaveRing::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    MoveSpeed += Acceleration * DeltaTime;

    float CurrentScaleX = GetActorScale3D().X;
    float MaxScale = 30.0f;

    FVector NewScale = GetActorScale3D() + FVector(MoveSpeed * DeltaTime);
    SetActorScale3D(NewScale);

    float OpacityAlpha = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, MaxScale), FVector2D(1.0f, 0.0f), CurrentScaleX);

    for (UMaterialInstanceDynamic* Mat : SliceMaterials)
    {
        if (Mat)
        {
            Mat->SetScalarParameterValue(FName("Opacity"), OpacityAlpha);
        }
    }

    if (CurrentScaleX > MaxScale)
    {
        Destroy();
    }
}