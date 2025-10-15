// InventoryComponent.cpp
#include "Item/Inventory/InventoryComponent.h"

#include "GameMode/PlayerState/InGamePlayerState.h"
#include "Item/ItemLog/ItemLog.h"
#include "Survivor/Character/AttributeComponent.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
    ConsumableSlots.Init(EItemType::None, 2);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UInventoryComponent, bHasKey);
    DOREPLIFETIME(UInventoryComponent, ConsumableSlots);
}

bool UInventoryComponent::AddItem(EItemType Item)
{
    if (!GetOwner() || Item == EItemType::None) return false;

    switch (Item)
    {
    case EItemType::Key:
        if (bHasKey) { return false; }
        bHasKey = true;
        OnItemAdded.Broadcast(Item);  
        OnInventoryUpdated.Broadcast(); 
        UE_LOG(LogItem, Log, TEXT("[Inventory] got KEY"));
        return true;

    case EItemType::Heal:
    case EItemType::Speed:
        for (int32 i=0;i<ConsumableSlots.Num();++i)
        {
            if (ConsumableSlots[i] == EItemType::None)
            {
                ConsumableSlots[i] = Item;
                OnItemAdded.Broadcast(Item);   // UI
                OnInventoryUpdated.Broadcast(); // UI
                UE_LOG(LogItem, Log, TEXT("[Inventory] slot%d <- %s"), i, *UEnum::GetValueAsString(Item));
                return true;
            }
        }
        UE_LOG(LogItem, Warning, TEXT("[Inventory] consumables full (2/2)"));
        return false;

    default: return false;
    }
}

bool UInventoryComponent::ConsumeKey()
{
    if (!bHasKey) { UE_LOG(LogItem, Warning, TEXT("[Inventory] no key to consume")); return false; }
    bHasKey = false; UE_LOG(LogItem, Log, TEXT("[Inventory] key consumed"));
    // UI 즉시 갱신
    OnInventoryUpdated.Broadcast();
    return true;
}

bool UInventoryComponent::UseItemBySlot(int32 SlotIndex)
{
    if (!ConsumableSlots.IsValidIndex(SlotIndex) || ConsumableSlots[SlotIndex] == EItemType::None)
    { UE_LOG(LogItem, Warning, TEXT("[Inventory] slot%d empty"), SlotIndex); return false; }

    const EItemType Item = ConsumableSlots[SlotIndex];

    // === 효과 적용  ===
    if (AActor* OwnerActor = GetOwner())
    {
        if (UAttributeComponent* Attr = OwnerActor->FindComponentByClass<UAttributeComponent>())
        {
            switch (Item)
            {
            case EItemType::Heal:
                {
                    const float HealAmount = 50.f;
                    const float NewHP = FMath::Clamp(Attr->GetHealth() + HealAmount, 0.f, Attr->MaxHealth);
                    Attr->SetHealth(NewHP);
                    UE_LOG(LogItem, Log, TEXT("[Use] Heal +%.0f -> %.0f/%.0f"), HealAmount, NewHP, Attr->MaxHealth);

                    // ✅ 이미 상위 스코프에 선언된 OwnerActor를 그대로 사용
                    APawn* OwnerPawn = Cast<APawn>(OwnerActor);
                    if (OwnerPawn)
                    {
                        APlayerState* PS = OwnerPawn->GetPlayerState();
                        if (AInGamePlayerState* InGamePS = Cast<AInGamePlayerState>(PS))
                        {
                            if (OwnerPawn->HasAuthority())
                                InGamePS->ServerSetSurvivorState(ESurvivorState::Alive);
                            else
                                InGamePS->ServerSetSurvivorState(ESurvivorState::Alive);

                            UE_LOG(LogItem, Log, TEXT("[Inventory] Heal used -> SurvivorState set to Alive (%s)"), *InGamePS->GetPlayerName());
                        }
                    }
                    break;
                }

            case EItemType::Speed:
            {
                const float AddSpeed = 200.f, Duration = 5.f;
                Attr->ApplySpeedBuff(AddSpeed, Duration);
                UE_LOG(LogItem, Log, TEXT("[Use] Speed +%.0f for %.1fs"), AddSpeed, Duration);
                break;
            }
            default: break;
            }
        }
    }
    // === 효과 끝 ===

    ConsumableSlots[SlotIndex] = EItemType::None; // 사용 후 비움
    OnItemUsed.Broadcast(Item, SlotIndex);    // 아이템 사용 이벤트
    OnInventoryUpdated.Broadcast(); // 전체 갱신 이벤트
    return true;
}
// OnRep 델리게이트 추가
void UInventoryComponent::OnRep_ConsumableSlots()
{
    OnInventoryUpdated.Broadcast();
}
void UInventoryComponent::OnRep_HasKey()
{
    OnInventoryUpdated.Broadcast();
}