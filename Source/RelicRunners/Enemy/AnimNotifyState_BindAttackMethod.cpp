// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotifyState_BindAttackMethod.h"
#include "Components/PrimitiveComponent.h"

void UAnimNotifyState_BindAttackMethod::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	UE_LOG(LogTemp, Warning, TEXT("Notify Begin Attack"));

	Owner = MeshComp->GetOwner();

	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("Owner not set"));
		return;
	}

	if (!TargetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Target Class Not Set"));
		return;
	}

	TArray<UPrimitiveComponent*> colliders; 
	Owner->GetComponents<UPrimitiveComponent>(colliders);
	
	FObjectProperty* objProp = CastField<FObjectProperty>(TargetClass->FindPropertyByName(*ColliderName));

	if (!objProp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Object property not found"));
		return;
	}

	TObjectPtr<UObject> uncastedCollider = objProp->GetObjectPropertyValue_InContainer(Owner);

	if (uncastedCollider)
	{
		UE_LOG(LogTemp, Warning, TEXT("Problem finding the collider"));
		return;
	}


	UPrimitiveComponent* collider = Cast<UPrimitiveComponent>(uncastedCollider);

	if (!collider)
	{
		UE_LOG(LogTemp, Warning, TEXT("Collider not found"));
		return;
	}

	colliderComp = collider;
	onOverlapDelegate.BindUFunction(Owner, FName(*AttackMethodName));
	
	if (collider->OnComponentBeginOverlap.Contains(onOverlapDelegate))
	{
		collider->OnComponentBeginOverlap.Remove(onOverlapDelegate);
	}

	collider->OnComponentBeginOverlap.Add(onOverlapDelegate);
	collider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void UAnimNotifyState_BindAttackMethod::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!Owner)
	{
		return;
	}

	if (!colliderComp)
	{
		return;
	}

	colliderComp->OnComponentBeginOverlap.Remove(onOverlapDelegate);
	colliderComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

#if WITH_EDITOR
void UAnimNotifyState_BindAttackMethod::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    FName PropertyName = NAME_None;

	if (PropertyChangedEvent.Property)
	{
		PropertyName = PropertyChangedEvent.Property->GetFName();
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UAnimNotifyState_BindAttackMethod, TargetClass))
	{
		AttackMethodName = "";
		ColliderName = "";
		MethodOptions.Empty();
		PropertyOptions.Empty();

		for (TFieldIterator<FObjectProperty> itProp = TFieldIterator<FObjectProperty >(TargetClass); itProp; ++itProp)
		{
			FObjectProperty* prop = *itProp;

			//prop->
			

			if (prop->PropertyClass->IsChildOf(UPrimitiveComponent::StaticClass()))
			{
				PropertyOptions.Add(prop->GetName());
			}
		}

		for (TFieldIterator<UFunction> itFunc = TFieldIterator<UFunction>(TargetClass); itFunc; ++itFunc)
		{
			UFunction* signature = GetClass()->FindFunctionByName("OnComponentBeginOverlapSignature");
			UFunction* func = *itFunc;

			if (signature->IsSignatureCompatibleWith(func))
			{
				MethodOptions.Add(func->GetName());
			}
		}
	}
}
#endif

TArray<FString> UAnimNotifyState_BindAttackMethod::GetMethodOptions() const
{
	return MethodOptions;
}

TArray<FString> UAnimNotifyState_BindAttackMethod::GetPropertyOptions() const
{
	return PropertyOptions;
}

void UAnimNotifyState_BindAttackMethod::OnComponentBeginOverlapSignature(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}
