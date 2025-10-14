#include "GameMode/Controller/TitlePlayerController.h"
#include "UI/Widgets/MainMenu/MainMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "MediaTexture.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"
#include "GameMode/GameInstance/EHGameInstance.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

ATitlePlayerController::ATitlePlayerController()
{
    bShowMouseCursor = false;
}

void ATitlePlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController())
        return;

    InitializeBackgroundVideoPlane();

    if (MP_Intro && MS_Intro && MT_Intro)
    {
        if (BackgroundMID)
        {
            BackgroundMID->SetTextureParameterValue(TEXT("VideoTexture"), MT_Intro);
        }
        MP_Intro->OnEndReached.AddDynamic(this, &ATitlePlayerController::OnIntroEnd);
        MP_Intro->OpenSource(MS_Intro);
        MP_Intro->Play();
    }
    else
    {
        OnIntroEnd();
    }
}

void ATitlePlayerController::ShowTitleUI_Local()
{
    if (MainMenuWidgetClass && !MainMenuInstance)
    {
        MainMenuInstance = CreateWidget<UMainMenuWidget>(this, MainMenuWidgetClass);
        if (MainMenuInstance)
        {
            MainMenuInstance->AddToViewport();

            MainMenuInstance->OnPlayClicked.AddDynamic(this, &ATitlePlayerController::HandlePlaySelected);
            MainMenuInstance->OnInputIPClicked.AddDynamic(this, &ATitlePlayerController::HandleInputIPSelected);
            MainMenuInstance->OnOptionsClicked.AddDynamic(this, &ATitlePlayerController::HandleOptionsSelected);
           
            MainMenuInstance->InitializeMenu();
            SetupUIInputMode(MainMenuInstance);
        }
    }
}

void ATitlePlayerController::HideOptionsMenu()
{

    if (UEHGameInstance* GI = Cast<UEHGameInstance>(GetGameInstance()))
    {
        GI->HideOptionsMenu();
    }

    if (MainMenuInstance)
    {
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(MainMenuInstance->TakeWidget());
        SetInputMode(InputMode);
        SetShowMouseCursor(false);
    }
    else
    {
        // 만약 메인 메뉴가 없다면, 새로 생성
        ShowTitleUI_Local();
    }
}

void ATitlePlayerController::HandleOptionsSelected()
{
    UE_LOG(LogTemp, Log, TEXT("PlayerController << RECEIVED BROADCAST: Options"));

    if (UEHGameInstance* GI = Cast<UEHGameInstance>(GetGameInstance()))
    {
        GI->ShowOptionsMenu();

        if (UUserWidget* OptionsMenu = GI->GetOptionsMenuInstance())
        {
            FInputModeUIOnly InputMode;
            InputMode.SetWidgetToFocus(OptionsMenu->TakeWidget());
            SetInputMode(InputMode);
            SetShowMouseCursor(true);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController << FAILED to Cast to EHGameInstance!"));
    }
}

void ATitlePlayerController::SetupUIInputMode(UUserWidget* FocusWidget)
{
    FInputModeUIOnly InputMode;

    if (FocusWidget)
    {
        InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
    }

    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);
    SetShowMouseCursor(true); 
}

#pragma region Unchanged Functions
void ATitlePlayerController::HandlePlaySelected()
{
    if (UEHGameInstance* GI = Cast<UEHGameInstance>(GetGameInstance()))
    {
        const FString TargetIP = GI->GetIPAddress();
        if (TargetIP.IsEmpty())
        {
            ShowMessagePopup(FText::FromString(TEXT("IP가 입력되지 않았습니다. Join 메뉴에서 IP를 먼저 입력해주세요.")));
        }
        else
        {
            if (MainMenuInstance) 
            { 
                MainMenuInstance->RemoveFromParent(); MainMenuInstance = nullptr; 
            }

            if (ZoomInVideoWidgetClass && !ZoomInVideoInstance) 
            { 
                ZoomInVideoInstance = CreateWidget<UUserWidget>(this, ZoomInVideoWidgetClass); 
                if (ZoomInVideoInstance) 
                { 
                    ZoomInVideoInstance->AddToViewport(100); 
                } 
            }

            if (MP_ZoomIn && MS_ZoomIn) 
            { 
                MP_ZoomIn->OnEndReached.AddDynamic(this, &ATitlePlayerController::OnZoomInEnd); 
                MP_ZoomIn->OpenSource(MS_ZoomIn); 
                MP_ZoomIn->Play(); 
            }
            else 
            { 
                OnZoomInEnd(); 
            }

        }
    }

}

void ATitlePlayerController::ShowIPInputPopup()
{
    if (IPInputPopupWidgetClass)
    {
        UUserWidget* IPPopup = CreateWidget<UUserWidget>(this, IPInputPopupWidgetClass);
        if (IPPopup)
        {
            IPPopup->AddToViewport(2);

            FInputModeUIOnly InputMode;
            SetInputMode(InputMode);

            SetShowMouseCursor(true);
        }
    }
}

// IP 입력 팝업을 닫고, 원래의 키보드 전용 모드로 되돌리는 함수
void ATitlePlayerController::CloseIPInputPopup()
{
    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, IPInputPopupWidgetClass);

    for (UUserWidget* Widget : FoundWidgets)
    {
        Widget->RemoveFromParent();
    }

    if (MainMenuInstance)
    {
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(MainMenuInstance->TakeWidget());
        SetInputMode(InputMode);
        SetShowMouseCursor(false);
    }
    else
    {
        ShowTitleUI_Local();
    }
}

void ATitlePlayerController::ShowMessagePopup(const FText& Message)
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Message.ToString());

    UE_LOG(LogTemp, Warning, TEXT("ShowMessagePopup: %s"), *Message.ToString());

}

void ATitlePlayerController::HandleInputIPSelected() 
{ 
    ShowIPInputPopup();
}

void ATitlePlayerController::OnIntroEnd()
{
    if (MP_Loop && MS_Loop && MT_Loop)
    {
        if (BackgroundMID) { BackgroundMID->SetTextureParameterValue(TEXT("VideoTexture"), MT_Loop); }
        MP_Loop->OpenSource(MS_Loop);
        MP_Loop->SetLooping(true);
        MP_Loop->Play();
    }

   
    ShowTitleUI_Local();
}

void ATitlePlayerController::OnZoomInEnd()
{
    if (ZoomInVideoInstance) 
    { 
        ZoomInVideoInstance->RemoveFromParent(); 
        ZoomInVideoInstance = nullptr; 
    
    }
    
    if (UEHGameInstance* GI = Cast<UEHGameInstance>(GetGameInstance()))
    {
        const FString TargetIP = GI->SavedIPAddress; 

        // IP 주소가 비어있지 않은지 한번 더 확인하고 접속을 시도
        if (!TargetIP.IsEmpty())
        {
            UE_LOG(LogTemp, Log, TEXT("OnZoomInEnd: Connecting to saved IP: %s"), *TargetIP);
            GI->ConnectToIP(TargetIP);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("OnZoomInEnd: Target IP is empty. Cannot connect."));
        }
    }
}

void ATitlePlayerController::InitializeBackgroundVideoPlane()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("BackgroundVideoPlane"), FoundActors);
    if (FoundActors.Num() > 0)
    {
        AActor* PlaneActor = FoundActors[0];
        UStaticMeshComponent* MeshComponent = PlaneActor->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComponent) { BackgroundMID = MeshComponent->CreateAndSetMaterialInstanceDynamic(0); }
    }
}