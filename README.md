# The Eleventh Hour – Team11 CH4 Project

## 프로젝트 소개

**The Eleventh Hour**는 **1 vs 5 비대칭 멀티플레이** 서바이벌 게임입니다.

* **1명의 살인마(Murderer)** 와 **5명의 생존자(Survivor)** 가 서로 다른 목표를 달성해야 합니다.
* 생존자는 **발전기 수리 → 키패드 입력 → 문 개방 → 탈출** 루트를 완수하면 승리합니다.
* 살인마는 생존자를 추격, 공격하여 탈출을 저지하고 제한 시간 내 전원을 제압하면 승리합니다.

언리얼 엔진 5.5.4와 C++을 기반으로 개발되었으며, **네트워크 멀티플레이 환경**을 전제로 설계되었습니다.

---

## 개발 환경

* **Engine** : Unreal Engine 5.5.4
* **IDE** : JetBrains Rider / Visual Studio 2022
* **Version Control** : Git + GitHub (branch 전략 기반 협업)
* **Language** : C++ (Enhanced Input, Replication, UMG UI 포함)

---

## 게임 규칙

### 생존자

* 맵 내 **발전기(Generators)** 수리
* **키패드(MultiKeypad)** 입력 → 문(KeypadDoor) 개방
* **열쇠(Key) 아이템**으로 특정 문 해제 가능
* 힐팩 / 스피드 버프 아이템을 사용해 생존 확률 증가
* 탈출구로 나가면 승리

### 살인마

* 기본 이동 속도가 생존자보다 빠름
* 근접 공격(애니메이션+Trace 판정)으로 생존자 제압
* 생존자 체력이 0이 되면 탈락
* 제한 시간 내 생존자 탈출 실패 시 살인마 승리

---

## 주요 기능

### 1. 게임 흐름 관리

* **GameMode**: `MultiGameModeBase`, `InGameGameMode`, `WaitingGameMode`
* **GameState / PlayerState**: 대기방 상태, 인게임 진행 상태, 플레이어 개별 상태 동기화

### 2. 캐릭터

* **생존자 (`EHSurvivorCharacter`)**

  * 이동, 앉기/걷기/공격/상호작용
  * `InventoryComponent` 연동: 열쇠 사용, 소모 아이템 사용 (Heal, Speed)
  * `AttributeComponent`: HP 및 이동 속도 관리
* **살인마 (`MurdererCharacter`)**

  * 근접 공격 애니메이션 및 Trace 판정
  * NetMulticast RPC 기반 공격 모션 동기화

### 3. 기믹 (Gimmicks)

* **Generators**: 협력 수리, 완료 시 GameMode 보고 → 키패드 활성화
* **MultiKeypad**: 발전기 완료 후 입력 가능, 완료 시 가장 가까운 `KeypadDoor` 개방
* **Doors**

  * `MultiplayDoor`: Timeline 애니메이션 기반 문 열림/닫힘
  * `KeypadDoor`: 키패드와 연동되어 열림

### 4. 아이템 시스템

* **Chest**: 파괴 시 랜덤 드랍 (열쇠, 힐, 스피드)
* **ItemPickup**: 일정 시간 뒤 획득 가능, 일정 시간 미획득 시 자동 소멸
* **ItemSpawnVolume**: 아이템/상자 스폰 및 리스폰 관리
* **InventoryComponent**

  * Key 1개 슬롯, Consumables 2개 슬롯
  * Heal 사용 시 체력 회복, Speed 사용 시 일정 시간 속도 버프

### 5. 애니메이션

* `EHAnimInstanceBase`: 생존자 이동/공격/상태 반영
* 살인마 공격 몽타주 실행 시 AnimNotify 기반 타격 판정

---

## 네트워킹 & 동기화

* 모든 주요 Actor(`Generators`, `Keypad`, `Door`, `Chest`, `ItemPickup`)는 `bReplicates = true`
* 상태 값(Progress, Health, Open 여부 등)은 `DOREPLIFETIME` + `OnRep_` 콜백으로 동기화
* 상호작용은 **Server RPC → Multicast RPC** 패턴 사용

---

## 프로젝트 구조

```
Source/TheEleventhHour
├── GameMode
│   ├── MultiGameModeBase / InGameGameMode / WaitingGameMode
│   └── InGamePlayerState / WaitingPlayerState / WaitingGameState
├── Survivor
│   ├── EHSurvivorCharacter / EHSurvivorController
│   └── EHAnimInstanceBase
├── Murderer
│   └── MurdererCharacter
├── Item
│   ├── Chest / ItemPickup / InventoryComponent / ItemSpawnVolume
│   └── ItemTypes / ItemLog
├── Gimmick
│   ├── Door (MultiplayDoor, KeypadDoor)
│   ├── Generators
│   └── Keypad (MultiKeypad)
```

---

## 팀 역할

| 이름      | 담당                                             |
| ------- | ---------------------------------------------- |
| 김선우     | 생존자 로직, 기믹 일부                                  |
| **이서호** | 아이템 시스템 + README 정리 |
| 서윤성     | GameMode / GameState / 네트워크 틀                  |
| 정윤호     | 살인마 캐릭터, 근접 공격                                 |
| 이정국     | UI / HUD                                       |
| 허중영     | 맵 디자인                                          |
---

## 시연 영상

> 추후 유튜브 링크 추가 예정

---

## 향후 확장

* 살인마/생존자 스킬 
* HUD 개선 (스코어, 체력바 등)
* 애니메이션 블렌드 강화

---
