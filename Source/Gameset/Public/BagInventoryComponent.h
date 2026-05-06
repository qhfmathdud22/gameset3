#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemDefinition.h"
#include "BagInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBagChanged);

// 격자에 놓인 아이템 하나
USTRUCT(BlueprintType)
struct FBagItemSlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) UItemDefinition* Item = nullptr;
	UPROPERTY(BlueprintReadOnly) int32 GridX = 0;
	UPROPERTY(BlueprintReadOnly) int32 GridY = 0;
};

UCLASS(ClassGroup = (Delivery), meta = (BlueprintSpawnableComponent))
class GAMESET_API UBagInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBagInventoryComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bag")
	int32 GridWidth = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bag")
	int32 GridHeight = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bag")
	float MaxWeight = 30.f;

	// 현재 격자에 놓인 아이템들
	UPROPERTY(BlueprintReadOnly, Category = "Bag")
	TArray<FBagItemSlot> PlacedItems;

	UPROPERTY(BlueprintAssignable, Category = "Bag")
	FOnBagChanged OnBagChanged;

	// ── 아이템 추가 (자동으로 빈자리 찾아서 배치) ───────────────────
	UFUNCTION(BlueprintCallable, Category = "Bag")
	bool TryAddItem(UItemDefinition* Item);

	// ── 아이템 제거 ─────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Bag")
	bool RemoveItemByIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Bag")
	void ClearAll();

	// ── 무게 ────────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Bag")
	float GetTotalWeight() const;

	UFUNCTION(BlueprintPure, Category = "Bag")
	float GetWeightRatio() const;

	// ── 격자 리사이즈 (업그레이드) ──────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Bag")
	void ResizeGrid(int32 NewWidth, int32 NewHeight, float NewMaxWeight);

	// 특정 셀이 어떤 아이템 인덱스에 의해 점유됐는지 (-1 = 빈칸)
	UFUNCTION(BlueprintPure, Category = "Bag")
	int32 GetItemIndexAt(int32 X, int32 Y) const;

private:
	bool CanPlaceAt(UItemDefinition* Item, int32 X, int32 Y) const;
};
