#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeliveryTypes.h"
#include "DeliveryPhoneWidget.generated.h"

// 전방 선언 (헤더 포함 최소화)
class UDeliveryAppWidget;
class UDeliveryNotifBannerWidget;
class UCanvasPanel;
class UBorder;
class UTextBlock;

/**
 * UDeliveryPhoneWidget
 *
 * 배달의 민족 스타일 휴대폰 위젯.
 * - 잠금화면 상단 실제 시간 / 게임 시간 표시
 * - 배달 앱 탭: Pending 주문 목록 표시, 수락/거절
 * - BP(블루프린트)에서 UI 비주얼을 담당하고,
 *   C++에서 데이터 접근 및 비즈니스 로직을 제공한다.
 *
 * [알림 아키텍처]
 * - NotifBannerOverlay : 뷰포트에 ZOrder=200으로 직접 추가된 독립 위젯
 *   → 폰 위젯 열림/닫힘 상태와 무관하게 항상 표시 가능
 * - DeliveryAppWidgetInstance : 뷰포트에 ZOrder=100으로 직접 추가
 *   → 전체화면 앱 UI, ✕ 버튼으로 닫기
 */
UCLASS()
class GAMESET_API UDeliveryPhoneWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ────────────────────────────────────────────────────────────────────
	// 실제 시간 (잠금화면 상단)
	// ────────────────────────────────────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "Phone|Time")
	FText GetRealTimeText() const;

	UFUNCTION(BlueprintPure, Category = "Phone|Time")
	FText GetRealDateText() const;

	// ────────────────────────────────────────────────────────────────────
	// 게임 시간 (17분 = 하루, 2026/04/19 오전 6시 시작)
	// ────────────────────────────────────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "Phone|GameTime")
	FText GetGameTimeText() const;   // 예: 오전 6:00

	UFUNCTION(BlueprintPure, Category = "Phone|GameTime")
	FText GetGameDateText() const;   // 예: 2026년 4월 19일 일요일

	// ────────────────────────────────────────────────────────────────────
	// 화면 상태
	// ────────────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadWrite, Category = "Phone")
	bool bScreenOpen = false;

	// ────────────────────────────────────────────────────────────────────
	// 유저가 BP에서 이미지만 설정하면 되는 UPROPERTY들
	// ────────────────────────────────────────────────────────────────────

	/** 배달 앱 아이콘 이미지 텍스처 (알림 배너 아이콘으로도 사용) */
	UPROPERTY(EditDefaultsOnly, Category = "DeliveryApp|Icons")
	UTexture2D* DeliveryAppIconTexture = nullptr;

	/** 알림 배너 전용 아이콘 이미지 텍스처 */
	UPROPERTY(EditDefaultsOnly, Category = "DeliveryApp|Icons")
	UTexture2D* NotificationIconTexture = nullptr;

	/** 앱 시그니처 주황색 (기본값: 배달의 민족 주황 #FF6B35) */
	UPROPERTY(EditDefaultsOnly, Category = "DeliveryApp|Colors")
	FLinearColor AppPrimaryColor = FLinearColor(1.f, 0.42f, 0.21f, 1.f);

	/** 새 배달 알림 효과음 (BP 클래스 디폴트에서 Sound Wave/Cue 할당) */
	UPROPERTY(EditDefaultsOnly, Category = "DeliveryApp|Audio")
	class USoundBase* NotificationSound = nullptr;

	/**
	 * 폰 블루프린트 안에서 배달 앱이 표시될 CanvasPanel.
	 * BP 디자이너에서 "PhoneAppContainer" 라는 이름으로 CanvasPanel을 추가하면
	 * 자동으로 연결되어 앱이 폰 화면 안에 표시됩니다.
	 * 없으면(nullptr) 전체 화면 뷰포트 오버레이로 대체됩니다.
	 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Phone")
	UCanvasPanel* PhoneAppContainer = nullptr;

	// ────────────────────────────────────────────────────────────────────
	// 잠금화면 알림 배지 (BP에서 이름을 정확히 맞춰야 자동 연결)
	// ────────────────────────────────────────────────────────────────────

	/**
	 * 잠금화면 시계 아래에 배치할 알림 배지 컨테이너.
	 * BP 디자이너에서 Border 위젯 이름을 "PhoneNotifBadge" 로 설정하면
	 * 새 배달 주문이 왔을 때 자동으로 표시/숨김 처리됩니다.
	 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Phone|Notification")
	UBorder* PhoneNotifBadge = nullptr;

	/**
	 * 알림 배지 안의 텍스트 위젯.
	 * BP 디자이너에서 TextBlock 이름을 "PhoneNotifText" 로 설정하세요.
	 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Phone|Notification")
	UTextBlock* PhoneNotifText = nullptr;

	/**
	 * 배달 앱 위젯 클래스.
	 * nullptr이면 기본 UDeliveryAppWidget을 사용.
	 * BP 서브클래스를 지정하면 해당 클래스로 생성됨.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "DeliveryApp")
	TSubclassOf<UDeliveryAppWidget> DeliveryAppWidgetClass;

	// C++에서 호출 → Blueprint에서 잠금화면으로 리셋 구현
	UFUNCTION(BlueprintImplementableEvent, Category = "Phone")
	void ResetToLockScreen();

	// ────────────────────────────────────────────────────────────────────
	// 배달 앱 관련 함수 (BlueprintImplementableEvent: BP에서 실제 UI 구현)
	// ────────────────────────────────────────────────────────────────────

	/** 새 주문 알림이 왔을 때 호출 → BP에서 알림 애니메이션 + 뱃지 표시 */
	UFUNCTION(BlueprintImplementableEvent, Category = "DeliveryApp")
	void OnNewOrderNotification(const FDeliveryOrder& Order);

	/**
	 * 배달 앱 위젯을 열고 Pending 주문 목록을 갱신한다.
	 * DeliveryAppWidgetInstance를 Visible로 전환.
	 */
	UFUNCTION(BlueprintCallable, Category = "DeliveryApp")
	void OpenDeliveryApp();

	/** 배달 앱 위젯을 닫는다 */
	UFUNCTION(BlueprintCallable, Category = "DeliveryApp")
	void CloseDeliveryApp();

	/** 주문 목록 새로고침 (Pending 오더 목록 변경 시 호출) */
	UFUNCTION(BlueprintImplementableEvent, Category = "DeliveryApp")
	void RefreshOrderList();

	// ────────────────────────────────────────────────────────────────────
	// C++에서 BP로 데이터를 제공하는 함수들
	// ────────────────────────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "DeliveryApp")
	TArray<FDeliveryOrder> GetPendingOrders() const;

	UFUNCTION(BlueprintCallable, Category = "DeliveryApp")
	bool AcceptDeliveryOrder(const FString& OrderID);

	UFUNCTION(BlueprintCallable, Category = "DeliveryApp")
	bool RejectDeliveryOrder(const FString& OrderID);

	UFUNCTION(BlueprintCallable, Category = "DeliveryApp")
	TArray<FDeliveryOrder> GetActiveOrders() const;

	/** 포맷된 보상 텍스트 반환. 예: "₩3,200" */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DeliveryApp")
	FText GetFormattedReward(float Amount) const;

	/** 포맷된 거리 텍스트 반환. 예: "1.2km" 또는 "850m" */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DeliveryApp")
	FText GetFormattedDistance(float Meters) const;

	/** 포맷된 시간 텍스트 반환. 예: "5분 30초" 또는 "45초" */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DeliveryApp")
	FText GetFormattedTime(float Seconds) const;

protected:
	virtual void NativeConstruct() override;

	/** 위젯이 화면에서 제거될 때 오버레이 위젯들도 정리 */
	virtual void NativeDestruct() override;

private:
	/** DeliveryComponent에서 게임 시간 읽기 */
	class UDeliveryComponent* GetDC() const;

	/** GameMode에서 DeliveryManagerComponent를 찾아 반환 */
	class UDeliveryManagerComponent* FindDeliveryManager() const;

	/**
	 * OnNewOrderPending 델리게이트 핸들러.
	 * 새 오더 도착 시 알림 배너 표시 + BP 이벤트 트리거.
	 */
	UFUNCTION()
	void HandleNewOrderPending(const FDeliveryOrder& Order);

	/** DeliveryManagerComponent 캐싱 */
	UPROPERTY()
	class UDeliveryManagerComponent* CachedDeliveryManager = nullptr;

	// ────────────────────────────────────────────────────────────────────
	// 독립 오버레이 위젯 (뷰포트에 직접 AddToViewport)
	// 폰 위젯 캔버스 밖에 있으므로 폰 열림/닫힘과 무관하게 표시됨
	// ────────────────────────────────────────────────────────────────────

	/** 알림 배너 오버레이 (ZOrder=200, 항상 최상단) */
	UPROPERTY()
	UDeliveryNotifBannerWidget* NotifBannerOverlay = nullptr;

	/** 배달 앱 전체화면 위젯 (ZOrder=100, 알림 아래) */
	UPROPERTY()
	UDeliveryAppWidget* DeliveryAppWidgetInstance = nullptr;

	/** 알림 배너 클릭 → 배달 앱 열기 */
	UFUNCTION()
	void OnNotificationClicked();

	/** 앱 위젯에서 수락이 눌렸을 때 호출 */
	UFUNCTION()
	void HandleAppOrderAccepted(const FString& OrderID);

	/** 앱 위젯에서 거절이 눌렸을 때 호출 */
	UFUNCTION()
	void HandleAppOrderRejected(const FString& OrderID);

	/** 알림 배너 위젯을 뷰포트에 생성 */
	void BuildNotificationOverlay();

	/** 배달 앱 위젯을 뷰포트에 생성 */
	void BuildDeliveryAppWidget();
};
