// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotifyState_BindAttackMethod.h"

void UAnimNotifyState_BindAttackMethod::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
#if WITH_EDITOR
	if (CallEditorDebugMessages)
	{
		UE_LOG(LogTemp, Log, TEXT("Notify Begin Attack"));
	}
#endif

	CleanUpData();

	//check for no nullptr
	AActor* owner = MeshComp->GetOwner();
	if (!owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("Owner not set"));
		return;
	}

	if (!TargetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Target Class Not Set"));
		return;
	}
	else if (!owner->IsA(TargetClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("Class and owner do not match"));
		return;
	}

	//get the collider and check it
	TArray<UPrimitiveComponent*> colliders; 
	owner->GetComponents<UPrimitiveComponent>(colliders);
	
	FObjectProperty* objProp = CastField<FObjectProperty>(TargetClass->FindPropertyByName(*ColliderName));

	if (!objProp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Object property not found"));
		return;
	}

	//not this can cause a error, if this does happen make sure to let me Thomas know and send me the error message and the breakpoint. This should be fixed though
	UObject* uncastedCollider = objProp->GetObjectPropertyValue_InContainer(owner);

	if (!uncastedCollider)
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

	FAttackNotifyStateData& data = ActiveData.FindOrAdd(MeshComp);

	data.ColliderComp = collider;

	//check to make sure their is overlap events
	if (!collider->GetGenerateOverlapEvents())
	{
		collider->SetGenerateOverlapEvents(true);
	}

	if (ModifiyCollider)
	{
		collider->SetCollisionEnabled(BeginCollisionType);
	}

	if (collider->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
	{
		UE_LOG(LogTemp, Warning, TEXT("Collision enable tpye is set to no collision, nothing will happen because of it"));
	}

	//bind the attack method
	data.onOverlapDelegate.BindUFunction(this, FName(TEXT("OnComponentBeginOverlapSignature")));
	if (!collider->OnComponentBeginOverlap.Contains(data.onOverlapDelegate))
	{
		collider->OnComponentBeginOverlap.Add(data.onOverlapDelegate);
		//collider->OnComponentBeginOverlap.Remove(onOverlapDelegate);
	}

	//collider->OnComponentBeginOverlap.Add(onOverlapDelegate);

	//check for already overlapping actors
	TArray<UPrimitiveComponent*> otherColliders;
	collider->GetOverlappingComponents(otherColliders);

	for (UPrimitiveComponent* otherCollider : otherColliders)
	{
		if (!otherCollider)
		{
			continue;
		}

		FHitResult blank;

		//the -1 is the part of a skelton mesh that was hit we can figure that out because of GetOverlappingComponents so just set it to -1
		OnComponentBeginOverlapSignature(collider, otherCollider->GetOwner(), otherCollider, -1, false, blank);
	}
}

void UAnimNotifyState_BindAttackMethod::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
#if WITH_EDITOR
	if (CallEditorDebugMessages)
	{
		UE_LOG(LogTemp, Log, TEXT("Notify End Attack"));
	}
#endif

	CleanUpData();

	FAttackNotifyStateData data;

	if (!ActiveData.RemoveAndCopyValue(MeshComp, data))
	{
		return;
	}

	if (!data.ColliderComp)
	{
		return;
	}

	if (DisableOverlapEvents)
	{
		data.ColliderComp->SetGenerateOverlapEvents(false);
	}

	if (ModifiyCollider)
	{
		data.ColliderComp->SetCollisionEnabled(EndCollisionType);
	}

	data.ColliderComp->OnComponentBeginOverlap.Remove(data.onOverlapDelegate);
	data.AlreadyHitActors.Empty();
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
	for (TPair< TWeakObjectPtr<USkeletalMeshComponent>, FAttackNotifyStateData>& Pair : ActiveData)
	{
		FAttackNotifyStateData& data = Pair.Value;

		if (OverlappedComponent == data.ColliderComp && OverlappedComponent->GetOwner() && OtherActor != OverlappedComponent->GetOwner() && !data.AlreadyHitActors.Contains(OtherActor))
		{
			UFunction* attackFunction = TargetClass->FindFunctionByName(*AttackMethodName);

			if (!attackFunction)
			{
				return;
			}

			//have to use a struct to pass in the parameters as on does not just simply call the function within a UFunction 
			struct AttackArguments
			{
				UPrimitiveComponent* OverlappedComponent;
				AActor* OtherActor;
				UPrimitiveComponent* OtherComp;
				int32 OtherBodyIndex;
				bool bFromSweep;
				FHitResult SweepResult;
			};

			AttackArguments argument;
			argument.OverlappedComponent = OverlappedComponent;
			argument.OtherActor = OtherActor;
			argument.OtherComp = OtherComp;
			argument.OtherBodyIndex = OtherBodyIndex;
			argument.bFromSweep = bFromSweep;
			argument.SweepResult = SweepResult;

			OverlappedComponent->GetOwner()->ProcessEvent(attackFunction, &argument);
			data.AlreadyHitActors.Add(OtherActor);
		}
	}
}

void UAnimNotifyState_BindAttackMethod::CleanUpData()
{
	for (auto It = ActiveData.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.RemoveCurrent(); 
		}
	}
}
