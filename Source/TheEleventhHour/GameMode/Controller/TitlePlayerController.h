#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TitlePlayerController.generated.h"

class UUserWidget;
class UMainMenuWidget;
class UMediaPlayer;
class UMediaSource;
class UMediaTexture;
class UMaterialInstanceDynamic;
class UMainMenuWidget;

UCLASS()
class THEELEVENTHHOUR_API ATitlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATitlePlayerController();

	virtual void BeginPlay() override;

	// 메인 메뉴 UI 출력
	UFUNCTION(BlueprintCallable, Category = "UI/Events")
	void ShowTitleUI_Local();
	
	// 메뉴에서 Play(Host) 클릭 시
	UFUNCTION(BlueprintCallable, Category = "UI/Events")
	void HandlePlaySelected();
	
	// 메뉴에서 Join 클릭 시
	UFUNCTION(BlueprintCallable, Category = "UI/Events")
	void HandleInputIPSelected();

	// 메뉴에서 Options 클릭 시
	UFUNCTION(BlueprintCallable, Category = "UI/Events")
	void HandleOptionsSelected();

	void HideOptionsMenu();

	// IP INPUT 팝업창 관련 함수
	UFUNCTION(BlueprintCallable)
	void ShowIPInputPopup();
	UFUNCTION(BlueprintCallable)
	void CloseIPInputPopup();
	UFUNCTION(BlueprintCallable)
	void ShowMessagePopup(const FText& Message); // 메시지 팝업용

	// 인트로 영상 종료 → 루프 영상으로 전환
	UFUNCTION()
	void OnIntroEnd();

	// 플레이 버튼 누른 뒤 줌인 영상 종료 → 다음 맵 이동
	UFUNCTION()
	void OnZoomInEnd();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
	TObjectPtr<UMediaPlayer> MP_Intro;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
	TObjectPtr<UMediaPlayer> MP_Loop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
	TObjectPtr<UMediaPlayer> MP_ZoomIn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media|Source")
	TObjectPtr<UMediaSource> MS_Intro; // 인트로 미디어 소스

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media|Source")
	TObjectPtr<UMediaSource> MS_Loop; // 배경 루프 미디어 소스

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media|Source")
	TObjectPtr<UMediaSource> MS_ZoomIn; // 줌인 미디어 소스

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media|Texture")
	TObjectPtr<UMediaTexture> MT_Intro;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media|Texture")
	TObjectPtr<UMediaTexture> MT_Loop;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;

	UPROPERTY()
	TObjectPtr<UMainMenuWidget> MainMenuInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> ZoomInVideoWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> ZoomInVideoInstance;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> IPInputPopupWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> MessagePopupWidgetClass;

	// 런타임 제어 -> 배경 영상 평면의 다이내믹 머티리얼 인스턴스
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BackgroundMID;

	// 배경 Plane 액터 -> 머티리얼을 설정 초기화 함수
	void InitializeBackgroundVideoPlane();

	// UI에 포커스를 맞추고 입력 모드를 설정하는 헬퍼 함수
	void SetupUIInputMode(UUserWidget* FocusWidget);
};