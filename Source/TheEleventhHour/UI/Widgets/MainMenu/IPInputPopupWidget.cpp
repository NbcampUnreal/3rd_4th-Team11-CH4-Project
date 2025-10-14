#include "IPInputPopupWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "GameMode/GameInstance/EHGameInstance.h" 
#include "GameMode/Controller/TitlePlayerController.h" 

void UIPInputPopupWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Button_Apply)
    {
        Button_Apply->OnClicked.AddDynamic(this, &UIPInputPopupWidget::OnApplyClicked);
    }

    if (Button_Cancel)
    {
        Button_Cancel->OnClicked.AddDynamic(this, &UIPInputPopupWidget::OnCancelClicked);
    }

    if (UEHGameInstance* GI = Cast<UEHGameInstance>(GetGameInstance()))
    {
        if (IP_InputBox)
        {
            IP_InputBox->SetText(FText::FromString(GI->SavedIPAddress));
        }
    }

}

void UIPInputPopupWidget::OnApplyClicked()
{
    if (UEHGameInstance* GI = Cast<UEHGameInstance>(GetGameInstance()))
    {
        if (IP_InputBox)
        {
            GI->SetIPAddress(IP_InputBox->GetText().ToString());
        }
    }

    if (ATitlePlayerController* PC = Cast<ATitlePlayerController>(GetOwningPlayer()))
    {
        PC->CloseIPInputPopup();
    }
}

void UIPInputPopupWidget::OnCancelClicked()
{
    if (ATitlePlayerController* PC = Cast<ATitlePlayerController>(GetOwningPlayer()))
    {
        PC->CloseIPInputPopup();
    }
}