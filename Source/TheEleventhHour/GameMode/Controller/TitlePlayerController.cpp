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

            MainMenuInstance->OnHostClicked.AddDynamic(this, &ATitlePlayerController::HandlePlaySelected);
            MainMenuInstance->OnJoinClicked.AddDynamic(this, &ATitlePlayerController::HandleJoinSelected);
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
    if (MainMenuInstance) { MainMenuInstance->RemoveFromParent(); MainMenuInstance = nullptr; }
    if (ZoomInVideoWidgetClass && !ZoomInVideoInstance) { ZoomInVideoInstance = CreateWidget<UUserWidget>(this, ZoomInVideoWidgetClass); if (ZoomInVideoInstance) { ZoomInVideoInstance->AddToViewport(100); } }
    if (MP_ZoomIn && MS_ZoomIn) { MP_ZoomIn->OnEndReached.AddDynamic(this, &ATitlePlayerController::OnZoomInEnd); MP_ZoomIn->OpenSource(MS_ZoomIn); MP_ZoomIn->Play(); }
    else { OnZoomInEnd(); }
}

void ATitlePlayerController::HandleJoinSelected() 
{ 
    if (UEHGameInstance* GI = Cast<UEHGameInstance>(GetGameInstance()))
    {
        // 임시 지정
        const FString LocalIP = TEXT("127.0.0.1");
        UE_LOG(LogTemp, Log, TEXT("[TitlePC] Attempting to connect to local IP: %s"), *LocalIP);
        GI->ConnectToIP(LocalIP);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[TitlePC] GameInstance cast failed"));
    }
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
    if (ZoomInVideoInstance) { ZoomInVideoInstance->RemoveFromParent(); ZoomInVideoInstance = nullptr; }
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    SetShowMouseCursor(true);
    UGameplayStatics::OpenLevel(GetWorld(), TEXT("WaitingLevel"), true);
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