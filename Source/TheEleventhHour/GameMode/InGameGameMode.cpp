#include "GameMode/InGameGameMode.h"
#include "GameMode/PlayerState/InGamePlayerState.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

AInGameGameMode::AInGameGameMode()
{
    bUseSeamlessTravel = true; // WaitingLevel → GameLevel 이동 시 PlayerState 유지
    PlayerStateClass = AInGamePlayerState::StaticClass();
    bRolesAssigned = false;

    // 🔹 BP 경로 직접 지정 (필요 시 블루프린트에서 덮어쓰기 가능)
    // static ConstructorHelpers::FClassFinder<APawn> KillerBP(TEXT("/Game/Blueprints/BP_Murderer"));
    // if (KillerBP.Succeeded()) KillerPawnClass = KillerBP.Class;
    //
    // static ConstructorHelpers::FClassFinder<APawn> SurvivorBP(TEXT("/Game/Blueprints/BP_Survivor"));
    // if (SurvivorBP.Succeeded()) SurvivorPawnClass = SurvivorBP.Class;
}

void AInGameGameMode::PostSeamlessTravel()
{
    Super::PostSeamlessTravel();
    UE_LOG(LogTemp, Log, TEXT("[InGameGM] PostSeamlessTravel -> AssignRoles"));

    if (HasAuthority() && !bRolesAssigned)
    {
        AssignRoles();           // 🔥 Pawn 스폰 전에 역할 먼저 부여
        RespawnKillerIfNeeded(); // 🔥 혹시 이미 잘못 스폰된 경우 Killer만 교체
        bRolesAssigned = true;
    }
}

void AInGameGameMode::AssignRoles()
{
    if (!HasAuthority() || !GameState) return;

    const TArray<APlayerState*>& AllPlayers = GameState->PlayerArray;
    if (AllPlayers.Num() == 0) return;

    const int32 KillerIndex = UKismetMathLibrary::RandomInteger(AllPlayers.Num());
    UE_LOG(LogTemp, Log, TEXT("[RoleAssign] Selecting KillerIndex: %d (Total: %d)"), KillerIndex, AllPlayers.Num());

    for (int32 i = 0; i < AllPlayers.Num(); ++i)
    {
        if (AInGamePlayerState* PS = Cast<AInGamePlayerState>(AllPlayers[i]))
        {
            PS->bIsKiller = (i == KillerIndex);
            UE_LOG(LogTemp, Log, TEXT("[RoleAssign] %s -> %s"),
                   *PS->GetPlayerName(),
                   PS->bIsKiller ? TEXT("KILLER") : TEXT("SURVIVOR"));

            PS->ForceNetUpdate(); // ✅ 즉시 클라이언트 동기화
        }
    }
}

/** 원하는 태그의 PlayerStart를 찾아 스폰 위치 반환 */
static void PickSpawnTransformByTag(UWorld* World, const FName DesiredTag, FTransform& OutTF, const AActor* FallbackStartSpot)
{
    APlayerStart* Picked = nullptr;

    // 1) 태그 매칭되는 PlayerStart 우선
    for (TActorIterator<APlayerStart> It(World); It; ++It)
    {
        if (It->PlayerStartTag == DesiredTag)
        {
            Picked = *It;
            break;
        }
    }

    // 2) 폴백: 엔진이 제공한 StartSpot 사용
    if (!Picked && FallbackStartSpot)
    {
        OutTF = FTransform(FallbackStartSpot->GetActorRotation(), FallbackStartSpot->GetActorLocation());
        return;
    }

    // 3) 폴백: 아무 PlayerStart 하나라도 사용
    if (!Picked)
    {
        for (TActorIterator<APlayerStart> It(World); It; ++It)
        {
            Picked = *It;
            break;
        }
    }

    // 4) 최종 폴백: 원점 약간 위로 올려 충돌 회피
    if (!Picked)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SpawnPick] No PlayerStart found. Using origin offset."));
        OutTF = FTransform(FRotator::ZeroRotator, FVector(0, 0, 200));
        return;
    }

    OutTF = FTransform(Picked->GetActorRotation(), Picked->GetActorLocation());
}

void AInGameGameMode::RespawnKillerIfNeeded()
{
    // 🔥 이미 Pawn이 잘못 스폰된 경우 Killer만 재포제스
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC) continue;

        if (AInGamePlayerState* PS = PC->GetPlayerState<AInGamePlayerState>())
        {
            if (PS->bIsKiller)
            {
                APawn* CurrentPawn = PC->GetPawn();
                if (!CurrentPawn || !CurrentPawn->IsA(KillerPawnClass))
                {
                    FTransform SpawnTF;
                    PickSpawnTransformByTag(GetWorld(), FName("Killer"), SpawnTF, CurrentPawn);

                    FActorSpawnParameters Params;
                    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

                    if (CurrentPawn) CurrentPawn->Destroy();

                    if (APawn* NewPawn = GetWorld()->SpawnActor<APawn>(
                        KillerPawnClass,
                        SpawnTF.GetLocation(),
                        SpawnTF.GetRotation().Rotator(),
                        Params))
                    {
                        PC->Possess(NewPawn);
                        UE_LOG(LogTemp, Log, TEXT("[Respawn] %s as Killer at %s"),
                            *PS->GetPlayerName(), *SpawnTF.GetLocation().ToString());
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("[Respawn] Killer spawn failed even with AdjustIfPossibleButAlwaysSpawn"));
                    }
                }
            }
        }
    }
}

APawn* AInGameGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
    if (AInGamePlayerState* PS = NewPlayer->GetPlayerState<AInGamePlayerState>())
    {
        const FName DesiredTag = PS->bIsKiller ? FName("Killer") : FName("Survivor");
        FTransform SpawnTF;
        PickSpawnTransformByTag(GetWorld(), DesiredTag, SpawnTF, StartSpot);

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        if (PS->bIsKiller && KillerPawnClass)
        {
            UE_LOG(LogTemp, Log, TEXT("[SpawnPawn] %s as KILLER at %s"),
                *PS->GetPlayerName(), *SpawnTF.GetLocation().ToString());

            if (APawn* P = GetWorld()->SpawnActor<APawn>(
                KillerPawnClass,
                SpawnTF.GetLocation(),
                SpawnTF.GetRotation().Rotator(),
                Params))
            {
                return P;
            }
            UE_LOG(LogTemp, Warning, TEXT("[SpawnPawn] Killer spawn failed. Fallback to Survivor."));
        }

        if (SurvivorPawnClass)
        {
            UE_LOG(LogTemp, Log, TEXT("[SpawnPawn] %s as SURVIVOR at %s"),
                *PS->GetPlayerName(), *SpawnTF.GetLocation().ToString());

            if (APawn* P = GetWorld()->SpawnActor<APawn>(
                SurvivorPawnClass,
                SpawnTF.GetLocation(),
                SpawnTF.GetRotation().Rotator(),
                Params))
            {
                return P;
            }
            UE_LOG(LogTemp, Error, TEXT("[SpawnPawn] Survivor spawn failed too at %s"), *SpawnTF.GetLocation().ToString());
        }
    }

    return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
}
