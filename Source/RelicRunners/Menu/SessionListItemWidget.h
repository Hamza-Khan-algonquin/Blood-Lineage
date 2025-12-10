// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "SessionListItemData.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "SessionListItemWidget.generated.h"


UCLASS()
class RELICRUNNERS_API USessionListItemWidget : public UUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    DECLARE_DELEGATE_OneParam(FOnSessionClicked, USessionListItemData*);
    FOnSessionClicked OnSessionClicked;

    virtual void NativeConstruct() override;
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

    UPROPERTY(meta = (BindWidget))
    class UButton* SessionButton;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* SessionNameText;
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* SessionPlayerCount;
    // Assign session data
    void Setup(class USessionListItemData* InData);

    void SetSelected(bool bInSelected);

private:
    UFUNCTION()
    void OnSessionButtonClicked();

    USessionListItemData* StoredSessionData;
};
