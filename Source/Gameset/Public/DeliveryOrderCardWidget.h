#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeliveryTypes.h"
#include "DeliveryOrderCardWidget.generated.h"

// 전방 선언 (헤더 포함 최소화)
class UTextBlock;
class UButton;
class UBorder;
class UVerticalBox;
class UHorizontalBox;
class UImage;
class UTexture2D;

/**
 * UDeliveryOrderCardWidget
 *
 * 배달 요청 한 건을 표시하는 카드 위젯.
 * Blueprint Designer 없이 C++ 코드만으로 레이아웃을 구성한다.
 * WidgetTree->ConstructWidget<T>() 패턴으로 모든 자식 위젯을 생성.
 */
UCLASS()
class GAMESET_API UDeliveryOrderCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ────────────────────────────────────────────────────────────────────
	// 델리게이트 선언 (수락/거절 버튼 이벤트)
	// ────────────────────────────────────────────────────────────────────

	/** 수락 버튼을 눌렀을 때 외부(DeliveryAppWidget)로 OrderID를 전달 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOrderAction, const FString&, OrderID);

	UPROPERTY(BlueprintAssignable, Category = "DeliveryCard|Events")
	FOnOrderAction OnAccepted;

	/** 거절 버튼을 눌렀을 때 외부(DeliveryAppWidget)로 OrderID를 전달 */
	UPROPERTY(BlueprintAssignable, Category = "DeliveryCard|Events")
	FOnOrderAction OnRejected;

	// ────────────────────────────────────────────────────────────────────
	// 퍼블릭 인터페이스
	// ────────────────────────────────────────────────────────────────────

	/**
	 * 카드에 표시할 주문 데이터를 세팅한다.
	 * BuildLayout() 이후 언제든지 호출 가능.
	 */
	UFUNCTION(BlueprintCallable, Category = "DeliveryCard")
	void SetOrderData(const FDeliveryOrder& Order);

	// ────────────────────────────────────────────────────────────────────
	// 외관 설정 (BP 에디터에서 음식점 아이콘만 교체 가능)
	// ────────────────────────────────────────────────────────────────────

	/** 음식점 아이콘 텍스처. nullptr이면 Image 위젯을 숨김 처리 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Appearance")
	UTexture2D* RestaurantIcon = nullptr;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	/**
	 * UUserWidget의 위젯 트리를 재빌드하는 오버라이드.
	 * C++에서 루트 위젯을 생성해 반환한다.
	 */
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	// ────────────────────────────────────────────────────────────────────
	// 캐시 데이터
	// ────────────────────────────────────────────────────────────────────

	/** SetOrderData()로 받은 주문 정보 캐싱 */
	FDeliveryOrder CachedOrder;

	// ────────────────────────────────────────────────────────────────────
	// 자식 위젯 포인터 (BuildLayout에서 생성 후 저장)
	// ────────────────────────────────────────────────────────────────────

	UPROPERTY()
	UTextBlock* RestaurantNameText = nullptr;   // 음식점 이름

	UPROPERTY()
	UTextBlock* ItemDescText = nullptr;          // 배달 물품 설명

	UPROPERTY()
	UTextBlock* RewardText = nullptr;            // 보상 금액 (주황색)

	UPROPERTY()
	UTextBlock* DistanceText = nullptr;          // 거리 표시

	UPROPERTY()
	UTextBlock* TimeRemainingText = nullptr;     // 남은 수락 시간

	UPROPERTY()
	UButton* AcceptBtn = nullptr;               // 수락 버튼

	UPROPERTY()
	UButton* RejectBtn = nullptr;               // 거절 버튼

	UPROPERTY()
	UImage* RestaurantImage = nullptr;          // 음식점 아이콘 이미지

	// ────────────────────────────────────────────────────────────────────
	// 내부 함수
	// ────────────────────────────────────────────────────────────────────

	/** UMG 위젯 트리 전체를 C++로 구성한다 (NativeConstruct에서 호출) */
	void BuildLayout();

	/** 수락 버튼 클릭 콜백 */
	UFUNCTION()
	void OnAcceptClicked();

	/** 거절 버튼 클릭 콜백 */
	UFUNCTION()
	void OnRejectClicked();

	// ────────────────────────────────────────────────────────────────────
	// 색상 상수
	// ────────────────────────────────────────────────────────────────────

	/** 배달 앱 시그니처 주황색 (#FF6B35) */
	static FLinearColor OrangeColor()   { return FLinearColor(1.f, 0.42f, 0.21f, 1.f); }

	/** 텍스트 회색 */
	static FLinearColor GrayColor()     { return FLinearColor(0.5f, 0.5f, 0.5f, 1.f); }

	/** 시간 표시 빨간색 */
	static FLinearColor RedColor()      { return FLinearColor(0.9f, 0.1f, 0.1f, 1.f); }

	/** 버튼 회색 배경 */
	static FLinearColor DarkGrayColor() { return FLinearColor(0.35f, 0.35f, 0.35f, 1.f); }
};
