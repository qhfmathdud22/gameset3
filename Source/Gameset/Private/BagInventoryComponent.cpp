#include "BagInventoryComponent.h"

UBagInventoryComponent::UBagInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// ── 무게 ──────────────────────────────────────────────────────────────

float UBagInventoryComponent::GetTotalWeight() const
{
	float Total = 0.f;
	for (const FBagItemSlot& S : PlacedItems)
		if (S.Item) Total += S.Item->Weight;
	return Total;
}

float UBagInventoryComponent::GetWeightRatio() const
{
	return MaxWeight > 0.f ? FMath::Clamp(GetTotalWeight() / MaxWeight, 0.f, 1.f) : 0.f;
}

// ── 배치 가능 판정 ────────────────────────────────────────────────────

bool UBagInventoryComponent::CanPlaceAt(UItemDefinition* Item, int32 X, int32 Y) const
{
	if (!Item) return false;

	// 격자 경계 체크
	if (X < 0 || Y < 0 || X + Item->GridWidth > GridWidth || Y + Item->GridHeight > GridHeight)
		return false;

	// 무게 체크
	if (GetTotalWeight() + Item->Weight > MaxWeight)
		return false;

	// 겹치는 아이템 체크
	for (const FBagItemSlot& S : PlacedItems)
	{
		if (!S.Item) continue;
		// X축 겹침
		bool bOverlapX = X < S.GridX + S.Item->GridWidth  && X + Item->GridWidth  > S.GridX;
		bool bOverlapY = Y < S.GridY + S.Item->GridHeight && Y + Item->GridHeight > S.GridY;
		if (bOverlapX && bOverlapY) return false;
	}
	return true;
}

// ── 아이템 추가 ───────────────────────────────────────────────────────

bool UBagInventoryComponent::TryAddItem(UItemDefinition* Item)
{
	if (!Item) return false;

	// 빈자리 자동 탐색 (왼쪽 위부터)
	for (int32 Y = 0; Y <= GridHeight - Item->GridHeight; ++Y)
	for (int32 X = 0; X <= GridWidth  - Item->GridWidth;  ++X)
	{
		if (CanPlaceAt(Item, X, Y))
		{
			FBagItemSlot Slot;
			Slot.Item  = Item;
			Slot.GridX = X;
			Slot.GridY = Y;
			PlacedItems.Add(Slot);
			OnBagChanged.Broadcast();
			return true;
		}
	}
	return false; // 빈자리 없음
}

// ── 아이템 제거 ───────────────────────────────────────────────────────

bool UBagInventoryComponent::RemoveItemByIndex(int32 Index)
{
	if (!PlacedItems.IsValidIndex(Index)) return false;
	PlacedItems.RemoveAt(Index);
	OnBagChanged.Broadcast();
	return true;
}

void UBagInventoryComponent::ClearAll()
{
	PlacedItems.Reset();
	OnBagChanged.Broadcast();
}

// ── 셀 조회 ──────────────────────────────────────────────────────────

int32 UBagInventoryComponent::GetItemIndexAt(int32 X, int32 Y) const
{
	for (int32 i = 0; i < PlacedItems.Num(); ++i)
	{
		const FBagItemSlot& S = PlacedItems[i];
		if (!S.Item) continue;
		if (X >= S.GridX && X < S.GridX + S.Item->GridWidth &&
		    Y >= S.GridY && Y < S.GridY + S.Item->GridHeight)
			return i;
	}
	return -1;
}

// ── 격자 리사이즈 ────────────────────────────────────────────────────

void UBagInventoryComponent::ResizeGrid(int32 NewWidth, int32 NewHeight, float NewMaxWeight)
{
	GridWidth  = FMath::Max(1, NewWidth);
	GridHeight = FMath::Max(1, NewHeight);
	MaxWeight  = FMath::Max(1.f, NewMaxWeight);

	// 새 격자 밖 아이템 제거
	for (int32 i = PlacedItems.Num() - 1; i >= 0; --i)
	{
		const FBagItemSlot& S = PlacedItems[i];
		if (!S.Item) { PlacedItems.RemoveAt(i); continue; }
		if (S.GridX + S.Item->GridWidth  > GridWidth ||
		    S.GridY + S.Item->GridHeight > GridHeight)
			PlacedItems.RemoveAt(i);
	}
	OnBagChanged.Broadcast();
}
