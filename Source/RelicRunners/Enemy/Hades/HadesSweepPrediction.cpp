#include "HadesSweepPrediction.h"
#include "Net/UnrealNetwork.h"
#include <Kismet/GameplayStatics.h>
#include "Engine/OverlapResult.h"
#include <RelicRunners/PlayerState/RelicRunnersPlayerState.h>

AHadesSweepPrediction::AHadesSweepPrediction()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void AHadesSweepPrediction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AHadesSweepPrediction, bIsRotating);
    DOREPLIFETIME(AHadesSweepPrediction, RotationCount);
    DOREPLIFETIME(AHadesSweepPrediction, StartYaw);
}

void AHadesSweepPrediction::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsRotating && Duration > 0.0f)
    {
        ElapsedTime += DeltaTime;
        float Alpha = FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f);

        float TotalDegrees = 360.0f * RotationCount;
        float CurrentAngle = FMath::Pow(Alpha, 4.0f) * TotalDegrees;

        FRotator NewRotation = FRotator(0, StartYaw - CurrentAngle, 0);
        SetActorRotation(NewRotation);

        if (AActor* Boss = GetOwner())
        {
            FVector BossLoc = FVector(0, 0, 0);
            FVector Offset = NewRotation.Vector() * 3450.0f;
            FVector FinalPos = BossLoc + Offset;
            FinalPos.Z = BossLoc.Z + 20.0f;
            SetActorLocation(FinalPos);

            if (HasAuthority())
            {
                FVector BoxHalfSize = FVector(950.0f, 250.0f, 200.0f);
                FCollisionShape Box = FCollisionShape::MakeBox(BoxHalfSize);

                TArray<FOverlapResult> Overlaps;
                FCollisionQueryParams Params;
                Params.AddIgnoredActor(Boss);
                Params.AddIgnoredActor(this);

                if (GetWorld()->OverlapMultiByChannel(Overlaps, GetActorLocation(), GetActorRotation().Quaternion(), ECC_Pawn, Box, Params))
                {
                    float CurrentTime = GetWorld()->GetTimeSeconds();

                    for (auto& Overlap : Overlaps)
                    {
                        if (AActor* HitActor = Overlap.GetActor())
                        {
                            if (!HitCooldownMap.Contains(HitActor) || CurrentTime >= HitCooldownMap[HitActor])
                            {
                                float FinalDamage = 25.0f; //Fallback of flat dmg

                                if (APawn* PlayerPawn = Cast<APawn>(HitActor))
                                {
                                    if (ARelicRunnersPlayerState* PS = PlayerPawn->GetPlayerState<ARelicRunnersPlayerState>())
                                    {
                                        //Calculate 20% of Max Health + 1% per elapsed second
                                        float BasePercent = PS->PlayerMaxHealth * 0.20f;
                                        float ScalingPercent = PS->PlayerMaxHealth * (ElapsedTime * 0.01f);

                                        FinalDamage = BasePercent + ScalingPercent;
                                    }
                                }

                                UGameplayStatics::ApplyDamage(HitActor, FinalDamage, nullptr, Boss, UDamageType::StaticClass());

                                HitCooldownMap.Emplace(HitActor, CurrentTime + 1.0f);
                            }
                        }
                    }
                }
            }
        }

        if (Alpha >= 1.0f)
        {
            bIsRotating = false;
            HitCooldownMap.Empty();
        }
    }
}