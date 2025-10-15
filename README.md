# The Eleventh Hour – Team11 CH4 Project

## 프로젝트 소개

**The Eleventh Hour**는 **1 vs 5 비대칭 멀티플레이** 서바이벌 게임입니다.

* **1명의 살인마(Murderer)** 와 **5명의 생존자(Survivor)** 가 서로 다른 목표를 달성해야 합니다.
* 생존자는 **발전기 수리 → 키패드 입력 → 문 개방 → 탈출** 루트를 완수하면 승리합니다. 발전기 전부 수리 시 키패드가 활성화되고, 문이 열리면 **남은 시간이 90초로 고정**됩니다.
* 살인마는 생존자를 추격·공격하여 **감옥** 으로 전원을 제압하거나 **제한시간**동안 플레이어들을 탈출하지 못하도록 막으면 승리합니다.

언리얼 엔진 5.5.4와 C++을 기반으로 개발되었으며, **네트워크 멀티플레이**를 전제로 설계되었습니다.

---

## 개발 환경

* **Engine** : Unreal Engine 5.5.4
* **IDE** : JetBrains Rider / Visual Studio 2022
* **Version Control** : Git + GitHub (branch 전략 기반 협업)
* **Language/Tech** : C++ (Enhanced Input, Replication, UMG UI)
* **Server / Hosting** : **AWS EC2** (리눅스 기반 전용 서버 빌드 실행, SeamlessTravel 사용)
  - 기본 구성: UE5 전용서버 바이너리 실행 → EC2 보안 그룹에서 게임 포트/Query 포트 허용

---
## 시연 영상

> 추후 유튜브 링크 추가 예정

---

## 게임 규칙

### 생존자

* 맵 내 **발전기(Generators)** 수리 → 수리 완료 수를 **GameMode → GameState**에 반영. 모든 발전기 완료 시 키패드가 활성화됩니다.
* **키패드(MultiKeypad)** 입력 성공 → **KeypadDoor(탈출문)** 개방 & **EscapeZone 활성화**, 남은 시간 **90초**로 설정.
* **열쇠(Key) 아이템**으로 특정 **감옥문(Prison Door)** 해제 가능(인벤토리에서 소모).
* **힐/스피드 버프** 소비형 아이템 사용 가능(숫자 1/2 슬롯). 스피드는 일정 시간 후 자동 원복.
* **탈출구**로 나가면 승리 판정에 반영됩니다.

### 살인마

* 근접 공격(서버 판정 Sweep)으로 생존자에게 피해를 주어 **Alive → Injured → Captured** 상태로 진행시킵니다. **포획 누적 3회**면 **Dead** 처리됩니다.
* 제한 시간 내 생존자 전원이 탈출하지 못하고 **Alive/Injured**가 0명이면 승리.

---

## 주요 기능

### 1) 게임 흐름 관리

* **Waiting(로비)**: Ready 인원/전체 인원 집계. **모두 Ready**면 **SeamlessTravel**로 인게임 맵으로 이동.
* **In-Game**: 플레이어 로드 완료 시 **역할(킬러 1명) 배정 → 5초 카운트다운 → 경기 시작**. 카운트 종료 시 **킬러 폰 교체**.
* **승패 판정**: 시간 초과 생존자 패배, 전원 포획/사망 시 킬러 승리, 탈출 성공 시 생존자 승리.

### 2) 캐릭터

* **생존자 (`AEHSurvivorCharacter`)**

  * 이동/시점/웅크리기/공격/상호작용(발전기·키패드·문) 입력 처리와 서버 RPC.
  * **InventoryComponent** 연동: **열쇠 소비(ConsumeKey)**, 소비형 아이템 슬롯(1/2).
  * **AttributeComponent**: HP/이동속도 레플리케이션, **ApplySpeedBuff**로 일정 시간 후 자동 원복.
  * 포획 상태 전환 시 서버에서 **PrisonManager**에 캡처 요청.
* **살인마 (`AMurdererCharacter`)**

  * 서버에서 공격 판정(Sweep)과 클라이언트 멀티캐스트 애니메이션 동기화(생존자 코드에 대응 로직 존재).

### 3) 기믹 (Gimmicks)

* **Generators**: 생존자 상호작용으로 수리 진행, 완료 시 **GameMode**가 전역 수리 수를 업데이트하고 **키패드 활성화**.
* **MultiKeypad**: 발전기 완료 후 상호작용 가능, 성공 시 탈출 루트로 연결.
* **Doors**

  * `MultiplayDoor`: Timeline 애니메이션으로 여닫기, 자동 닫힘. 감옥문 닫힐 때 **수감자 강제 이동(탈출 방향)** 훅 포함.
  * `KeypadDoor`: 열리면 **InGameGameMode.OnEscapeDoorOpened()** 호출 → EscapeZone 활성화 & 남은 시간 **90초**.

### 4) 아이템 시스템

* **Chest**: 파괴 시 랜덤 드랍(열쇠/힐/스피드). *(드랍/소멸 로직 전송 코드 기준)*
* **ItemPickup**: 일정 시간 뒤 획득 가능, 일정 시간 미획득 시 자동 소멸. *(전송 코드 기준)*
* **ItemSpawnVolume**: 아이템/상자 스폰 및 리스폰 관리. *(전송 코드 기준)*
* **InventoryComponent**: **Key 1칸 + Consumable 2칸** 구조, **OnInventoryUpdated** 델리게이트로 UI 갱신. *(전송 코드 기준)*

### 5) 애니메이션

* `EHAnimInstanceBase`: 이동/낙하/웅크리기/사망 여부 등 **상태 파생 값** 계산 및 애님 그래프 구동.
* 피격/사망 몽타주 멀티캐스트, 문 상호작용/일반 상호작용 몽타주 제공.

---

## 네트워킹 & 동기화

* 모든 주요 액터는 복제 활성화(`bReplicates=true`), 상태 값은 `DOREPLIFETIME` + `OnRep_`로 동기화.
* 상호작용은 **Server RPC → Multicast** 패턴을 사용(공격, 상호작용 애니 동기화 등).
* **SeamlessTravel**을 사용하여 로비→인게임 전환 시 컨트롤러/상태를 자연스럽게 유지.
* 클라이언트 전용 처리(입력 매핑, 일부 위젯 생성)는 **로컬 컨트롤러**에서만 수행.

---
## 기술 명세
[기술 명세서](https://docs.google.com/spreadsheets/d/1X9ahm_G0EAVkqSXg1_4ljJKRXitkZee3n3AqvlMyyu0/edit?gid=0#gid=0)

**파트별 클래스 설계**

| 이름   | 파트                     | 링크                         |
|-------|--------------------------|------------------------------|
| 김선우 | 생존자 로직 & 기믹 일부  | [생존자 클래스 다이어 그램](https://www.notion.so/28d345149799802a83c3f5ca4a3bb49f?source=copy_link) |
| 이서호 | 아이템 시스템 & README   | [아이템 클래스 다이어 그램](https://github.com/dltjgh17/Unreal_TIL/tree/main/October/10.14) |
| 서윤성 | GameMode / 네트워크 틀   | [서버, 게임모드 구조도](https://yoonsung.tistory.com/55) |                           |
| 정윤호 | 살인마 & 근접 공격       | [기술 정리](https://hakunamatata0930.tistory.com/63) |
| 이정국 | UI / HUD                | [Velog 글](https://velog.io/@palebluedot_/...Team-Project) |
| 허중영 | 맵 디자인                | [Tistory 글](https://gamergo.tistory.com/63) |
---

## 프로젝트 구조

```
Source/TheEleventhHour
├── GameMode
│   ├── MultiGameModeBase / InGameGameMode / WaitingGameMode
│   ├── GameState (InGameGameState / WaitingGameState)
│   ├── PlayerState (InGamePlayerState)
│   └── Controller (Title / Waiting / InGame)
├── Survivor
│   ├── Character (EHSurvivorCharacter)
│   ├── Controller (EHSurvivorController)
│   └── Animation (EHAnimInstanceBase)
├── Murderer
│   └── MurdererCharacter
├── Gimmick
│   ├── Door (MultiplayDoor, KeypadDoor)
│   ├── Generators
│   └── Keypad (MultiKeypad)
├── Item
│   ├── Chest / ItemPickup / ItemSpawnVolume
│   ├── Inventory (InventoryComponent)
│   └── ItemTypes / ItemLog
└── UI
    ├── InGameHUD (MatchTimer / MatchStatus / TeamStatus / Item / Skill)
    └── Title/Waiting/Result/Options/Menu 등
```

* 스폰: PlayerStart 중 **`Spawn.Player` 태그**가 붙은 지점에서 **중복 없는 랜덤** 배치.
* 타이틀: **BackgroundVideoPlane** 태그 액터의 메시에 동적 MID로 인트로/루프 영상을 출력.

---

## 팀 역할

| 이름      | 담당                                       |
| ------- | ---------------------------------------- |
| 김선우     | 생존자 로직, 기믹 일부 + 발표자료 정리                  |
| 이서호 | 아이템 시스템 + README 정리                      |
| 서윤성     | GameMode / GameState / 네트워크 틀 + 시연 영상 제작 |
| 정윤호     | 살인마 캐릭터, 근접 공격 + 기술 명세                   |
| 이정국     | UI / HUD + 발표                            |
| 허중영     | 맵 디자인 + 발표자료 정리                          |

---


## 향후 확장

* 살인마/생존자 **능동 스킬** 추가(쿨타임/자원 시스템)
* HUD 개선(팀 상태, 상세 통계, 알림 피드) 및 **Result** 화면에 **개인·팀별 KPI** 표시(수리/문해제/감금/킬 등)
* 애니메이션 블렌드/상태 동기화 강화, **옵션 저장(SaveGame) 영속화**, 인벤토리/아이템 튜토리얼 UX 보강

---

### 부록) 조작(기본)

**캐릭터**
* 상호작용: **E**(설정값 기준) - 발전기/키패드/문, 감옥문은 **열쇠 보유** 및 **Alive/Injured 상태**일 때만 가능.
* 아이템 사용: **1(슬롯0), 2(슬롯1)**.
* 벽넘기 : **F**.
* 점프 : **Spacebar**.
* 앉기 : **Ctrl**.
* 공격 : **마우스 좌클릭** - 상자 이외 공격 불가.

**살인마**
* 생존자 위치 확인 : **Q**.
* 점프 : **Spacebar**.
* 벽넘기 : **F**.
* 질주 : **Shift**
* 공격 : **마우스 좌클릭**
---
