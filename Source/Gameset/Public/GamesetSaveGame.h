#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GamesetSaveGame.generated.h"

/**
 * 게임 진행 데이터를 저장하는 SaveGame 클래스.
 * 슬롯 이름: "GamesetSave", 유저 인덱스: 0
 *
 * 저장 항목:
 *   - 소지금(Money), 게임 내 날짜(GameDay)
 *   - 배고픔(Hunger), 갈증(Thirst)
 *   - 업그레이드 이름 → 레벨 맵
 *   - 누적 배달 완료 횟수(TotalDelivered)
 */
UCLASS()
class GAMESET_API UGamesetSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// ── 슬롯 정보 ────────────────────────────────────────────────────
	/** 세이브 슬롯 이름 (LoadGameFromSlot / SaveGameToSlot 에서 키로 사용) */
	static const FString SaveSlotName;

	/** 유저 인덱스 (싱글플레이어는 항상 0) */
	static const int32 UserIndex;

	// ── 저장할 데이터 ─────────────────────────────────────────────────

	/** 플레이어 소지금 */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	float SavedMoney = 0.f;

	/** 게임 내 현재 날짜 (0부터 시작) */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	int32 SavedGameDay = 0;

	/**
	 * 업그레이드 이름 → 레벨 맵
	 * 키 예시: "SpeedBoost", "StaminaMax", "MultiOrder", "TimeBonus",
	 *          "TipMultiplier", "Sunglasses", "Shoes"
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	TMap<FString, int32> UpgradeLevels;

	/** 배고픔 (0~100, 100 = 배부름) */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	float SavedHunger = 100.f;

	/** 갈증 (0~100, 100 = 수분 충분) */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	float SavedThirst = 100.f;

	/** 누적 배달 완료 횟수 (게임 클리어 조건 판정에 사용) */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	int32 SavedTotalDelivered = 0;
};
