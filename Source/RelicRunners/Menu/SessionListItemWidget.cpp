
#include "SessionListItemWidget.h"
#include "SessionListItemData.h"
#include "Components/Button.h"

void USessionListItemWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (SessionButton)
    {
        SessionButton->OnClicked.AddDynamic(this, &USessionListItemWidget::OnSessionButtonClicked);
    }
}

void USessionListItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    USessionListItemData* SessionData = Cast<USessionListItemData>(ListItemObject);
    if (SessionData)
    {
        Setup(SessionData);
    }
}

void USessionListItemWidget::Setup(USessionListItemData* InData)
{
    StoredSessionData = InData;

    if (SessionNameText && StoredSessionData)
    {
        SessionNameText->SetText(FText::FromString(StoredSessionData->SessionName));
        SessionPlayerCount->SetText(FText::FromString(FString::FromInt(StoredSessionData->SessionPlayerCount)));
    }
}

void USessionListItemWidget::OnSessionButtonClicked()
{
    if (StoredSessionData && OnSessionClicked.IsBound())
    {
        OnSessionClicked.Execute(StoredSessionData);
    }
}

void USessionListItemWidget::SetSelected(bool bInSelected)
{
    if (SessionNameText)
    {
        if (bInSelected)
        {
            SessionNameText->SetColorAndOpacity(FSlateColor(FLinearColor(1, 0.5f, 0, 1)));
            SessionPlayerCount->SetColorAndOpacity(FSlateColor(FLinearColor(1, 0.5f, 0, 1)));
        }
        else
        {
            SessionNameText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
            SessionPlayerCount->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        }
    }
}