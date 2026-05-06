#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DeliveryTypes.h"
#include "DeliveryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMoneyChanged,     float, NewAmount, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged,   float, Current,   float, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnOrderCountChanged, int32, Count);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnSprintStateChanged, bool, bCanSprint);
// 배고픔 또는 갈증이 0이 됐을 때 게임 오버를 알리는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOver);

UCLASS(ClassGroup=(Delivery), meta=(BlueprintSpawnableComponent))
class GAMESET_API UDeliveryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDeliveryComponent();

	// 17분 = 게임 하루
	static constexpr float GameDayDuration = 1020.f;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ── Input ────────────────────────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Delivery")
	class UInputAction* PhoneOpenAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Delivery")
	class UInputAction* PhoneCloseAction;

	// TAB → 가방 인벤토리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Delivery")
	class UInputAction* InventoryAction;

	// ── 위젯 클래스 ──────────────────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UUserWidget> PhoneWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UUserWidget> InventoryWidgetClass;

	// ── Economy ──────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Economy") float Money = 0.f;
	UPROPERTY(BlueprintAssignable, Category = "Events") FOnMoneyChanged OnMoneyChanged;

	UFUNCTION(BlueprintCallable, Category = "Economy") void AddMoney(float Amount);
	UFUNCTION(BlueprintCallable, Category = "Economy") bool SpendMoney(float Amount);

	// ── 3가지 생존 스탯 ──────────────────────────────────────────────
	// 모두 0~100, 100 = 꽉 참 / 0 = 고갈

	// 스태미나
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Stamina")
	float BaseMaxStamina = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Stats|Stamina") float MaxStamina     = 100.f;
	UPROPERTY(BlueprintReadOnly, Category = "Stats|Stamina") float CurrentStamina = 100.f;
	UPROPERTY(EditDefaultsOnly,  Category = "Stats|Stamina") float StaminaDrainRate = 20.f;
	UPROPERTY(EditDefaultsOnly,  Category = "Stats|Stamina") float StaminaRegenRate = 12.f;

	UPROPERTY(BlueprintReadWrite, Category = "Stats|Stamina") bool  bIsSprinting     = false;
	UPROPERTY(BlueprintReadOnly, Category = "Stats|Stamina") bool  bSprintLocked    = false;
	// 이 속도(cm/s) 이상이면 달리기로 판단 (걷기 ~300, 달리기 ~600 사이값)
	UPROPERTY(EditDefaultsOnly, Category = "Stats|Stamina") float SprintDetectSpeed = 450.f;
	UPROPERTY(BlueprintReadOnly, Category = "Stats|Stamina") float SprintLockTimer  = 0.f;
	UPROPERTY(EditDefaultsOnly,  Category = "Stats|Stamina") float SprintLockDuration = 3.f;

	// 배고픔 (100 = 배부름)
	UPROPERTY(BlueprintReadOnly, Category = "Stats|Hunger")  float Hunger          = 100.f;
	UPROPERTY(EditDefaultsOnly,  Category = "Stats|Hunger")  float HungerDrainRate = 0.35f;  // /sec
	UPROPERTY(EditDefaultsOnly,  Category = "Stats|Hunger")  float HungerMax       = 100.f;

	// 갈증 (100 = 수분 충분)
	UPROPERTY(BlueprintReadOnly, Category = "Stats|Thirst")  float Thirst          = 100.f;
	UPROPERTY(EditDefaultsOnly,  Category = "Stats|Thirst")  float ThirstDrainRate = 0.55f;  // /sec (더 빨리 마름)
	UPROPERTY(EditDefaultsOnly,  Category = "Stats|Thirst")  float ThirstMax       = 100.f;

	UPROPERTY(BlueprintAssignable, Category = "Events") FOnStaminaChanged     OnStaminaChanged;
	UPROPERTY(BlueprintAssignable, Category = "Events") FOnSprintStateChanged OnSprintStateChanged;
	// 배고픔/갈증이 0이 될 때 한 번 발동 — BP에서 게임 오버 화면 등에 바인딩
	UPROPERTY(BlueprintAssignable, Category = "Events") FOnGameOver           OnGameOver;

	// 스프린트 가능 여부 (캐릭터 BP CanSprint 함수에서 AND 조건으로 사용)
	UFUNCTION(BlueprintPure, Category = "Stats") bool CanSprint() const;

	// 스프린트 상태 설정 — 캐릭터 BP 또는 입력 핸들러에서 호출
	UFUNCTION(BlueprintCallable, Category = "Stats") void SetSprinting(bool bSprint);

	// MaxWalkSpeed의 80%를 SprintDetectSpeed로 갱신 — UpgradeComponent에서 속도 업그레이드 후 호출
	UFUNCTION(BlueprintCallable, Category = "Stats") void UpdateSprintThreshold();

	// 음식/음료 소비 (인벤에서 우클릭, 픽업 시)
	UFUNCTION(BlueprintCallable, Category = "Stats") void EatFood(float Nutrition);
	UFUNCTION(BlueprintCallable, Category = "Stats") void DrinkWater(float Hydration);

	// ── 게임 시계 (17분=하루, 2026/04/19 오전 6시 시작) ──────────────
	UPROPERTY(BlueprintReadOnly, Category = "GameClock") float GameTimeSeconds = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "GameClock") int32 GameDay         = 0;

	// 게임 내 현재 시각 (시) 0~24 float
	UFUNCTION(BlueprintPure, Category = "GameClock")
	float GetGameHour() const { return (GameTimeSeconds / GameDayDuration) * 24.f; }

	// ── Delivery ─────────────────────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Delivery")
	int32 MaxCarryOrders = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Delivery")
	TArray<class APackageActor*> CarriedPackages;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnOrderCountChanged OnOrderCountChanged;

	UFUNCTION(BlueprintCallable, Category = "Delivery")
	void Interact();

	// ── 업그레이드 레벨 캐시 ─────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Upgrades") int32 SunglassesLevel = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Upgrades") int32 ShoesLevel      = 0;

	// 신발 레벨에 따른 달리기 속도 배율 (1.0 + 10%/레벨)
	UFUNCTION(BlueprintPure, Category = "Upgrades")
	float GetShoesSpeedMultiplier() const { return 1.f + ShoesLevel * 0.10f; }

	// 무게에 따른 속도 페널티 (가방 꽉 차면 50%)
	UFUNCTION(BlueprintPure, Category = "Upgrades")
	float GetWeightSpeedPenalty() const;

	// 최종 스프린트 속도 배율 = 신발 × 무게페널티
	UFUNCTION(BlueprintPure, Category = "Upgrades")
	float GetTotalSpeedMultiplier() const { return GetShoesSpeedMultiplier() * GetWeightSpeedPenalty(); }

	// ── 상태 조회 ────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool HasNearbyPackage() const { return NearbyPackages.Num() > 0; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool HasNearbyDropoff() const { return NearbyDropoffs.Num() > 0; }

	void AddNearbyPackage   (class APackageActor*       Package);
	void RemoveNearbyPackage(class APackageActor*       Package);
	void AddNearbyDropoff   (class ADeliveryPointActor* Dropoff);
	void RemoveNearbyDropoff(class ADeliveryPointActor* Dropoff);

	void ApplyStaminaUpgrade(float Multiplier);

private:
	TArray<TWeakObjectPtr<class APackageActor>>       NearbyPackages;
	TArray<TWeakObjectPtr<class ADeliveryPointActor>> NearbyDropoffs;

	UPROPERTY() class UUserWidget* PhoneWidgetInstance     = nullptr;
	UPROPERTY() class UUserWidget* InventoryWidgetInstance = nullptr;

	bool bInputBound        = false;
	bool bPhoneVisible      = false;
	bool bInventoryVisible  = false;
	bool bLastCanSprint     = true;   // 델리게이트 변화 감지용
	bool bGameOverTriggered = false;  // 게임 오버 델리게이트 중복 발동 방지

	void TryBindInput();
	void ShowPhone();
	void HidePhone();
	void ToggleInventory();

	void UpdateStats    (float DeltaTime);
	void UpdateGameClock(float DeltaTime);
	void TryPickupPackage();
	void TryDeliverPackage();

	void SetUIInputMode(bool bUIActive);
	class APlayerController* GetPC() const;
};
