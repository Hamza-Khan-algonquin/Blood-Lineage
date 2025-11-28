#include "Moonbeam.h"

#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h" 
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

#include "RelicRunners/Enemy/EnemyCharacter.h"
#include "RelicRunners/Enemy/EnemyCharacterAI.h"


AMoonbeam::AMoonbeam()
{ 
    AbilityName = FName(TEXT("Moonbeam"));
    
    // Tuning
    DamageAmount = 20.f;      
    Cooldown = 5.f;         

    // Beam visuals (will be adding more detailed visuals later)
    BeamRange = 1500.f;
    BeamRadius = 6.f;
    BeamDuration = 5.0f;

    bIsOnCooldown = false;
}

bool AMoonbeam::CanActivate() const
{
    return !bIsOnCooldown;
}

void AMoonbeam::ActivateAbility()
{
    if (!GetWorld())
    {
        return;
    }
   
    AActor* LocalOwner = OwnerActor ? OwnerActor : GetOwner();
    ACharacter* OwnerChar = Cast<ACharacter>(LocalOwner);

    FVector Start;
    FRotator ViewRot;

    if (OwnerChar->GetController())
    {
        OwnerChar->GetController()->GetPlayerViewPoint(Start, ViewRot);
    }
    else
    {
        OwnerChar->GetActorEyesViewPoint(Start, ViewRot);
    }

    const FVector Dir = ViewRot.Vector();
    const FVector End = Start + Dir * BeamRange;

    SpawnBeamEffect(Start, End);

    if (OwnerChar->HasAuthority())
    {
        FCollisionQueryParams Params(SCENE_QUERY_STAT(MoonbeamTrace), false);
        Params.AddIgnoredActor(OwnerChar);
        Params.AddIgnoredActor(this);

        const FVector TraceDir = (End - Start).GetSafeNormal();
        FVector TraceStart = Start;
        float RemainingDistance = BeamRange;

        while (RemainingDistance > 0.f)
        {
            FHitResult Hit;
            const FVector TraceEnd = TraceStart + TraceDir * RemainingDistance;

            if (!GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Pawn, Params))
            {
                break; 
            }

            AActor* HitActor = Hit.GetActor();
            if (!HitActor)
            {
                break;
            }

            if (HitActor && (HitActor->IsA<AEnemyCharacterAI>() || HitActor->IsA<AEnemyCharacter>()))
            {
                UGameplayStatics::ApplyDamage(HitActor, DamageAmount, OwnerChar->GetController(), OwnerChar, nullptr);
            }

            Params.AddIgnoredActor(HitActor);           
        }
    }

    // This part might change later becuase I know antonio is making changes in the ability system so will see
    bIsOnCooldown = true;
    if (Cooldown > 0.f)
    {
        GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle,this,&AMoonbeam::ResetCooldown,Cooldown,false);
    }
}

void AMoonbeam::EndAbility()
{
    // This is for later when adding for example some animition when make it end here and the effects end here too
}

void AMoonbeam::ResetCooldown()
{
    bIsOnCooldown = false;
}

void AMoonbeam::SpawnBeamEffect(const FVector& Start, const FVector& End)
{
    if (!GetWorld())
    {
        return;
    }

    // This line is to controll the visual of the beam that shows when pressing the ability button. For now it's just a debug line but later it will have more functionality
    // These values can be changed at the top 
    // I used this by adding DrawDebugHelpers.h (great library)
    DrawDebugLine(GetWorld(),Start,End,FColor::Cyan,false,BeamDuration,0,BeamRadius);
    
}
