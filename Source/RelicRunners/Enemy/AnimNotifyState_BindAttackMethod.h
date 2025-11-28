// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_BindAttackMethod.generated.h"


UCLASS(Blueprintable)
class RELICRUNNERS_API UAnimNotifyState_BindAttackMethod : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AActor* Owner;
	UPROPERTY(BlueprintReadOnly)
	class UPrimitiveComponent* colliderComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UClass* TargetClass;
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> MethodOptions;
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> PropertyOptions;

	UPROPERTY(DisplayName = "Attack Method", EditAnywhere, BlueprintReadOnly, meta = (GetOptions = "GetMethodOptions"))
	FString AttackMethodName;
	UPROPERTY(DisplayName = "Collider Property", EditAnywhere, BlueprintReadOnly, meta = (GetOptions = "GetPropertyOptions"))
	FString ColliderName;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FScriptDelegate onOverlapDelegate;

public:
	virtual void NotifyBegin(class USkeletalMeshComponent* MeshComp, class UAnimSequenceBase* Animation, float TotalDuration) override;

	virtual void NotifyEnd(class USkeletalMeshComponent* MeshComp, class UAnimSequenceBase* Animation) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	UFUNCTION()
	TArray<FString> GetMethodOptions() const;
	UFUNCTION()
	TArray<FString> GetPropertyOptions() const;
private:
	UFUNCTION()
	void OnComponentBeginOverlapSignature(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
