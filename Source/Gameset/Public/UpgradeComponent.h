#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DeliveryTypes.h"
#include "UpgradeComponent.generated.h"

USTRUCT(BlueprintType)
struct FUpgradeLevel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)  int32 Level     = 0;
	UPROPERTY(EditDefaultsOnly)   int32 MaxLevel  = 3;
	UPROPERTY(EditDefaultsOnly)   FText DisplayName;
	UPROPERTY(EditDefaultsOnly)   FText Description;
	UPROPERTY(EditDefaultsOnly)   float BaseCost  = 500.f;
	UPROPERTY(EditDefaultsOnly)   float CostScale = 1.5f;

	float GetNextCost() const { return BaseCost * FMath::Pow(CostScale, (float)Level); }
	bool  CanUpgrade()  const { return Level < MaxLevel; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradePurchased, EUpgradeType, Type);

UCLASS(ClassGroup = (Delivery), meta = (BlueprintSpawnableComponent))
class GAMESET_API UUpgradeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UUpgradeComponent();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrades")
	TMap<EUpgradeType, FUpgradeLevel> Upgrades;

	// 기본 걷기 속도 (SpeedBoost와 Shoes가 이를 기준으로 곱하기 적용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float BaseWalkSpeed = 400.f;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUpgradePurchased OnUpgradePurchased;

	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	bool TryPurchaseUpgrade(EUpgradeType Type);

	UFUNCTION(BlueprintPure, Category = "Upgrades")
	int32 GetUpgradeLevel(EUpgradeType Type) const;

	UFUNCTION(BlueprintPure, Category = "Upgrades")
	float GetUpgradeCost(EUpgradeType Type) const;

	UFUNCTION(BlueprintPure, Category = "Upgrades")
	bool CanPurchaseUpgrade(EUpgradeType Type) const;

	float GetSpeedMultiplier()     const;
	float GetStaminaMultiplier()   const;
	int32 GetMaxOrders()           const;
	float GetTimeBonusSec()        const;
	float GetRewardMultiplier()    const;

private:
	void InitDefaults();
	void ApplyUpgrade(EUpgradeType Type);

	// 현재 모든 업그레이드를 반영하여 이동속도 재계산
	void RecalculateWalkSpeed();
};
