#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GamesetGameMode.generated.h"

// ── 델리게이트 선언 ────────────────────────────────────────────────────
/** 게임 오버 발생 시 브로드캐스트 — Reason: 사망 원인 문자열 (BP에서 UI에 연결) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameOverDelegate,  const FString&, Reason);

/** 게임 클리어 시 브로드캐스트 (BP에서 결과 화면 등에 연결) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameClearDelegate);

/**
 * 배달 게임 전용 GameMode.
 *
 * 담당 기능:
 *   - DeliveryManagerComponent 소유
 *   - GameOver / GameClear 트리거 및 델리게이트 브로드캐스트
 *   - SaveGame / LoadGame (슬롯: "GamesetSave")
 *   - 배달 완료 누적 횟수가 GameClearDeliveryCount에 도달하면 자동 클리어
 */
UCLASS()
class GAMESET_API AGamesetGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGamesetGameMode();

protected:
	virtual void BeginPlay() override;

public:
	// ── DeliveryManager ──────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Delivery")
	class UDeliveryManagerComponent* DeliveryManager;

	// ── 게임 클리어 조건 ─────────────────────────────────────────────
	/**
	 * 누적 배달 완료 횟수가 이 값 이상이 되면 GameClear 발동.
	 * BP에서 자유롭게 수정 가능.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameRules")
	int32 GameClearDeliveryCount = 10;

	// ── 델리게이트 ───────────────────────────────────────────────────
	/** BP(UI위젯 등)에서 바인딩해 게임 오버 화면을 띄울 때 사용 */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGameOverDelegate OnGameOver;

	/** BP(UI위젯 등)에서 바인딩해 게임 클리어 화면을 띄울 때 사용 */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGameClearDelegate OnGameClear;

	// ── GameOver / GameClear ─────────────────────────────────────────
	/**
	 * 게임 오버를 발동시킨다.
	 * @param Reason  원인 문자열 (예: "배고픔 고갈", "갈증 고갈")
	 */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void TriggerGameOver(const FString& Reason);

	/** 게임 클리어를 발동시킨다. */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void TriggerGameClear();

	// ── SaveGame / LoadGame ──────────────────────────────────────────
	/**
	 * 현재 게임 상태(소지금·허기·갈증·업그레이드 레벨 등)를 슬롯에 저장.
	 * 배달 완료(NotifyOrderDelivered) 이후 자동 호출되며 BP에서도 직접 호출 가능.
	 */
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void SaveGame();

	/**
	 * 슬롯에서 게임 상태를 불러와 플레이어에게 적용.
	 * 저장 데이터가 없으면 기본값으로 시작.
	 * BeginPlay()에서 자동 호출.
	 */
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void LoadGame();

private:
	// ── 내부 유틸리티 ────────────────────────────────────────────────
	/** 플레이어 Pawn에서 DeliveryComponent를 찾아 반환 (없으면 nullptr) */
	class UDeliveryComponent* GetDeliveryComponent() const;

	/** 플레이어 Pawn에서 UpgradeComponent를 찾아 반환 (없으면 nullptr) */
	class UUpgradeComponent* GetUpgradeComponent() const;

	/**
	 * DeliveryComponent::OnGameOver(파라미터 없음)에 바인딩되는 핸들러.
	 * 어떤 스탯이 고갈됐는지 판별 후 TriggerGameOver를 호출한다.
	 */
	UFUNCTION()
	void OnDeliveryComponentGameOver();

	/** DeliveryManager의 TotalDelivered가 목표치에 도달했는지 검사 후 클리어 */
	void CheckGameClearCondition();

	/**
	 * DeliveryManager의 OnNewOrderCreated 등 이벤트를 통해
	 * 배달 완료 시 자동 저장 + 클리어 조건을 체크하는 내부 핸들러.
	 * NotifyOrderDelivered 이후 DeliveryManager에서 호출되도록 바인딩됨.
	 *
	 * 현재 구현: BeginPlay에서 DeliveryManager::OnOrderDeliveredInternal 을
	 * 직접 바인딩하는 대신, Tick 또는 래퍼를 통해 처리.
	 * (DeliveryManagerComponent에 델리게이트가 없으므로 Tick 폴링 방식 사용)
	 */
	int32 LastKnownTotalDelivered = 0;

	/** 게임 오버/클리어가 이미 발동됐는지 여부 (중복 방지) */
	bool bGameEndTriggered = false;

protected:
	virtual void Tick(float DeltaSeconds) override;
};
