// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/PrimitiveComponent.h"
#include "AnimNotifyState_BindAttackMethod.h"

void UAnimNotifyState_BindAttackMethod::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Owner = MeshComp->GetOwner();

	if (!Owner)
	{
		return;
	}

	colliderComp = Owner->GetComponentByName
}

void UAnimNotifyState_BindAttackMethod::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!Owner)
	{
		return;
	}
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
