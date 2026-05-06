#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeliveryTypes.generated.h"

// ── 업그레이드 종류 ──────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
	SpeedBoost    UMETA(DisplayName = "Speed Boost"),
	StaminaMax    UMETA(DisplayName = "Stamina Pack"),
	MultiOrder    UMETA(DisplayName = "Extra Bag"),
	TimeBonus     UMETA(DisplayName = "Time Bonus"),
	TipMultiplier UMETA(DisplayName = "Tip Master"),
	Sunglasses    UMETA(DisplayName = "AR Sunglasses"),
	Shoes         UMETA(DisplayName = "Running Shoes"),
};

// ── 오더 상태 ────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EOrderStatus : uint8
{
	Pending    UMETA(DisplayName = "Pending"),    // 폰 알림 대기 중 (수락/거절 전)
	Available  UMETA(DisplayName = "Available"),  // 수락됨 (진행 중)
	PickedUp   UMETA(DisplayName = "Picked Up"),  // 픽업 완료
	Delivered  UMETA(DisplayName = "Delivered"),  // 배달 완료
	Failed     UMETA(DisplayName = "Failed"),      // 시간 초과 실패
	Rejected   UMETA(DisplayName = "Rejected"),   // 라이더가 거절함
};

// ── 배달 오더 ────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct GAMESET_API FDeliveryOrder
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FString OrderID;
	UPROPERTY(BlueprintReadOnly) FText   CustomerName;
	UPROPERTY(BlueprintReadOnly) float   Reward           = 0.f;
	UPROPERTY(BlueprintReadOnly) float   TimeLimit         = 120.f;
	UPROPERTY(BlueprintReadOnly) float   TimeRemaining     = 120.f;
	UPROPERTY(BlueprintReadOnly) EOrderStatus Status       = EOrderStatus::Pending;
	UPROPERTY(BlueprintReadOnly) AActor* PickupActor       = nullptr;
	UPROPERTY(BlueprintReadOnly) AActor* DropoffActor      = nullptr;

	// ── 배달의 민족 스타일 추가 정보 ─────────────────────────────────
	UPROPERTY(BlueprintReadOnly) float   DistanceMeters    = 0.f;   // 픽업→드롭오프 거리(m)
	UPROPERTY(BlueprintReadOnly) FText   ItemDescription;            // 배달 물품 설명 (예: 치킨 2마리 + 콜라)
	UPROPERTY(BlueprintReadOnly) FText   PickupName;                 // 픽업 장소 이름 (예: 맛있닭 강남점)
	UPROPERTY(BlueprintReadOnly) FText   DropoffName;                // 배달지 이름 (예: 김민준 고객)
	UPROPERTY(BlueprintReadOnly) float   PendingExpireTime  = 30.f;  // 수락 안 하면 30초 후 자동 소멸
	UPROPERTY(BlueprintReadOnly) float   PendingTimeRemaining = 30.f; // 폰 앱 타이머 표시용
};

// ── DREDGE 스타일 아이템 형태 (2D 셀 마스크) ─────────────────────────
// Width × Height 만큼의 bool 배열 (row-major). Cells[Y * Width + X]
USTRUCT(BlueprintType)
struct GAMESET_API FItemShape
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Width  = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Height = 1;

	// true = 차지하는 칸
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<bool> Cells;

	bool GetCell(int32 X, int32 Y) const
	{
		if (X < 0 || X >= Width || Y < 0 || Y >= Height) return false;
		const int32 Idx = Y * Width + X;
		return Cells.IsValidIndex(Idx) && Cells[Idx];
	}
};

// ── 인벤토리 아이템 정의 ─────────────────────────────────────────────
USTRUCT(BlueprintType)
struct GAMESET_API FInventoryItemDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName  ItemID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText  DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FItemShape Shape;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float  Weight     = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) class UTexture2D* Icon = nullptr;

	// 음식/음료 소비 시 배고픔·갈증 회복량
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float  Nutrition  = 0.f;  // 배고픔 회복
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float  Hydration  = 0.f;  // 갈증 회복
};

// ── 인벤토리 안의 배치된 아이템 (회전 포함 확장 버전) ──────────────────
// BagInventoryComponent.h의 FBagItemSlot과 이름 충돌 방지를 위해 FBagPlacedItem으로 명명
USTRUCT(BlueprintType)
struct GAMESET_API FBagPlacedItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite) FInventoryItemDef Item;
	UPROPERTY(BlueprintReadWrite) int32 GridX    = 0;
	UPROPERTY(BlueprintReadWrite) int32 GridY    = 0;
	UPROPERTY(BlueprintReadWrite) int32 Rotation = 0;   // 0/1/2/3 (×90°)
};
