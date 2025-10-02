#include "GameMode/Controller/TitlePlayerController.h"
#include "GameMode/UI/TitleUserWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void ATitlePlayerController::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("[TitlePC] BeginPlay executed on %s (Local=%d)"),
           *GetNameSafe(this),
           IsLocalController() ? 1 : 0);

    if (!IsLocalController())
        return;

    bShowMouseCursor = true;
    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);

    // UI 생성
    Client_ShowTitleUI();
}

void ATitlePlayerController::Client_ShowTitleUI_Implementation()
{
    if (TitleUserWidgetClass && !TitleUserWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("[TitlePC] Creating Title UI on client side: %s"), *GetNameSafe(this));

        bShowMouseCursor = true;
        FInputModeGameAndUI InputMode;
        InputMode.SetHideCursorDuringCapture(false);
        SetInputMode(InputMode);

        TitleUserWidget = CreateWidget<UTitleUserWidget>(this, TitleUserWidgetClass);
        if (TitleUserWidget)
        {
            TitleUserWidget->AddToViewport();
        }
    }
}