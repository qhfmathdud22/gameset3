#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.generated.h"

UCLASS(BlueprintType)
class GAMESET_API UItemDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	// ── 이것만 설정하면 끝! ──────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	// ── 격자에서 차지하는 칸 수 (기본 1×1) ──────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Grid")
	int32 GridWidth = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Grid")
	int32 GridHeight = 1;

	// ── 선택 옵션 ────────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stats")
	float Weight = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stats")
	float Nutrition = 0.f;   // 배고픔 회복

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stats")
	float Hydration = 0.f;   // 갈증 회복
};
