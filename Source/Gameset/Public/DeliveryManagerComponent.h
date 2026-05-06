#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DeliveryTypes.h"
#include "DeliveryManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewOrderCreated, const FDeliveryOrder&, Order);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOrderExpired,    const FString&,        OrderID);
// 새 배달 요청 도착 — 폰 앱 알림 UI 표시용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewOrderPending, const FDeliveryOrder&, Order);

// GameMode에 붙어서 오더를 생성하고 타이머를 관리
UCLASS(ClassGroup = (Delivery), meta = (BlueprintSpawnableComponent))
class GAMESET_API UDeliveryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDeliveryManagerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

public:
	// 레벨에 배치한 픽업/드롭오프 액터 목록 (에디터에서 할당)
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Setup")
	TArray<class APackageActor*> PickupPoints;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Setup")
	TArray<class ADeliveryPointActor*> DropoffPoints;

	/** 게임 시작 후 첫 오더가 뜨기까지의 딜레이 (초). 기본 2초 — 위젯 바인딩 대기용 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float InitialSpawnDelay = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float OrderSpawnInterval = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	int32 MaxActiveOrders = 3;

	// 동시에 폰 앱에 표시할 최대 Pending 오더 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Delivery")
	int32 MaxPendingOrders = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float BaseTimeLimit = 120.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float BaseReward = 200.f;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 TotalDelivered = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 TotalFailed = 0;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNewOrderCreated OnNewOrderCreated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnOrderExpired OnOrderExpired;

	// 새 배달 요청 도착 — 폰 앱 UI에서 수신하여 알림 표시
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNewOrderPending OnNewOrderPending;

	UFUNCTION(BlueprintCallable, Category = "Delivery")
	void TrySpawnNewOrder();

	UFUNCTION(BlueprintPure, Category = "Delivery")
	// 값 복사 대신 const 레퍼런스 반환 — 대형 배열 복사 비용 제거
	const TArray<FDeliveryOrder>& GetActiveOrders() const { return ActiveOrders; }

	// 폰 앱 UI용: 수락 대기 중인 오더 목록 반환
	UFUNCTION(BlueprintCallable, Category = "Delivery")
	const TArray<FDeliveryOrder>& GetPendingOrders() const { return PendingOrders; }

	// 폰 앱에서 오더 수락 — true 반환 시 성공
	UFUNCTION(BlueprintCallable, Category = "Delivery")
	bool AcceptOrder(const FString& OrderID);

	// 폰 앱에서 오더 거절 — true 반환 시 성공
	UFUNCTION(BlueprintCallable, Category = "Delivery")
	bool RejectOrder(const FString& OrderID);

	// DeliveryPawn: 배달을 완료한 플레이어 Pawn (UpgradeComponent 조회용)
	void NotifyOrderDelivered(const FString& OrderID, APawn* DeliveryPawn = nullptr);

private:
	TArray<FDeliveryOrder> ActiveOrders;
	// 수락 대기 중인 오더 목록 (폰 앱 알림 큐)
	TArray<FDeliveryOrder> PendingOrders;
	float         SpawnTimer          = 0.f;
	int32         OrderCounter        = 0;
	FTimerHandle  InitSpawnTimerHandle;   // BeginPlay 초기 스폰 타이머 (멤버로 유지해야 취소 가능)
	TArray<FText> CustomerNames;

	void UpdateTimers(float DeltaTime);
	FString GenerateOrderID();
	// DeliveryPawn을 받아 UpgradeComponent의 RewardMultiplier를 보상에 적용
	float   CalcReward(class APackageActor* Pickup, class ADeliveryPointActor* Dropoff, APawn* DeliveryPawn = nullptr) const;
};
