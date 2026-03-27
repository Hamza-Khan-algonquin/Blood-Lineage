#pragma once

#include "CoreMinimal.h"
#include "Engine/DamageEvents.h"
#include "Templates/SubclassOf.h"
#include "Engine/NetSerialization.h"
#include "CustomDamageEvents.generated.h"

class UDamageType;

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	LIGHT_ATTACK = 0   UMETA(DisplayName = "Light Attack"),
	HEAVY_ATTACK = 1   UMETA(DisplayName = "Melee"),
	UNKOWN = 2         UMETA(DisplayName = "UNKNOWN")
};

USTRUCT(BlueprintType)
struct RELICRUNNERS_API FAttackDamageEvent : public FDamageEvent
{
	GENERATED_BODY()

	public: 
		EAttackType AttackType;
		float StunDuration;
		float KnockBackStrength;
		float AdditionalUpwardsKnockBackForces;
	
		FAttackDamageEvent() : FDamageEvent(), AttackType(EAttackType::UNKOWN), StunDuration(0.0f), KnockBackStrength(0.0f), AdditionalUpwardsKnockBackForces(0.0f) { }

		FAttackDamageEvent(EAttackType attackType, float knockBackStrength, float additionalUpwardsKnockBackForces) : FDamageEvent(), AttackType(attackType), StunDuration(0.0f), KnockBackStrength(knockBackStrength), AdditionalUpwardsKnockBackForces(additionalUpwardsKnockBackForces) {}

		FAttackDamageEvent(EAttackType attackType, float knockBackStrength, float additionalUpwardsKnockBackForces, TSubclassOf<UDamageType> inDamageTypeClass) : FDamageEvent(inDamageTypeClass), AttackType(attackType), StunDuration(0.0f), KnockBackStrength(knockBackStrength), AdditionalUpwardsKnockBackForces(additionalUpwardsKnockBackForces) { }

		virtual ~FAttackDamageEvent() { }

		static const int32 ClassID = 3;

		virtual int32 GetTypeID() const { return FAttackDamageEvent::ClassID; }
		virtual bool IsOfType(int32 InID) const { return FAttackDamageEvent::ClassID == InID; };
};