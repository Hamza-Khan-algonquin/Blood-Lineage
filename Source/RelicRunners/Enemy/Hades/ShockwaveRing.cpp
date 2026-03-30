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
    if (OtherActor && OtherActor != GetOwner() && OtherActor->IsA(ARelicRunnersCharacter::StaticClass()))
    {
        UGameplayStatics::ApplyDamage(OtherActor, 20.0f * Difficulty, GetInstigatorController(), this, UDamageType::StaticClass());
    }
}

void AShockwaveRing::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    MoveSpeed += Acceleration * DeltaTime;

    FVector NewScale = GetActorScale3D() + FVector(MoveSpeed * DeltaTime);
    SetActorScale3D(NewScale);

    if (GetActorScale3D().X > 40.0f)
    {
        Destroy();
    }
}