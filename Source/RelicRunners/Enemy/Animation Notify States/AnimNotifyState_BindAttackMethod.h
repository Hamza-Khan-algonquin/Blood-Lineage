// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Components/PrimitiveComponent.h"
#include "AnimNotifyState_BindAttackMethod.generated.h"

USTRUCT()
struct FAttackNotifyStateData
{
	GENERATED_BODY()

	UPROPERTY()
	UPrimitiveComponent* ColliderComp = nullptr;

	UPROPERTY()
	TArray<AActor*> AlreadyHitActors;

	FScriptDelegate onOverlapDelegate;
};

UCLASS(Blueprintable)
class RELICRUNNERS_API UAnimNotifyState_BindAttackMethod : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UClass* TargetClass;
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> MethodOptions;
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> PropertyOptions;

	UPROPERTY(DisplayName = "Attack Method", EditAnywhere, BlueprintReadOnly, Category = "Attack Bind", meta = (GetOptions = "GetMethodOptions"))
	FString AttackMethodName;
	UPROPERTY(DisplayName = "Collider Property", EditAnywhere, BlueprintReadOnly, Category = "Attack Bind", meta = (GetOptions = "GetPropertyOptions"))
	FString ColliderName;
	UPROPERTY(DisplayName = "Disable collider's on begin overlap events on notify state end", EditAnywhere, BlueprintReadOnly, Category = "Attack Bind")
	bool DisableOverlapEvents = false;
	UPROPERTY(DisplayName = "Should Modifiy Collision Type", EditAnywhere, BlueprintReadOnly, Category = "Attack Bind")
	bool ModifiyCollider = false;
	UPROPERTY(DisplayName = "Collision Type During Notify State", EditAnywhere, BlueprintReadOnly, Category = "Attack Bind", meta = (EditCondition = "ModifiyCollider", ToolTip = "In order for it to work you must have Querys enabled"))
	TEnumAsByte <ECollisionEnabled::Type> BeginCollisionType = ECollisionEnabled::QueryOnly;
	UPROPERTY(DisplayName = "Collision Type After Notify State", EditAnywhere, BlueprintReadOnly, Category = "Attack Bind", meta = (EditCondition = "ModifiyCollider"))
	TEnumAsByte <ECollisionEnabled::Type> EndCollisionType = ECollisionEnabled::NoCollision;

	//unlike WITH_EDITOR, WITH_EDITORONLY_DATA will actually remove the variable from builds not just the code
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Bind")
	bool CallEditorDebugMessages = true;
#endif

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
	void CleanUpData();

	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FAttackNotifyStateData> ActiveData;
};
