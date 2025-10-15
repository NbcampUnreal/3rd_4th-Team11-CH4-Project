#include "GameMode/GameInstance/PlayerSaveGame.h"

UPlayerSaveGame::UPlayerSaveGame()
{
    
    // 볼륨: 기본값 100 (내부적으로는 1.0f)
    MasterVolume = 1.0f;

    // 감마(밝기): 표준 감마값 2.2f 기본값
    Gamma = 2.2f;

    // 마우스 감도: 기본값 50 (내부적으로는 0.5f)
    MouseSensitivity = 0.5f;

    // 저장 파일의 이름과 인덱스를 설정
    SaveSlotName = TEXT("GameSettings"); // 파일 이름 -> "GameSettings.sav"
    UserIndex = 0; // 첫 번째(0번) 슬롯을 사용

}