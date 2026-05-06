#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeliveryTypes.h"
#include "DeliveryAppWidget.generated.h"

// 전방 선언
class UScrollBox;
class UTextBlock;
class UBorder;
class UVerticalBox;
class UHorizontalBox;
class UButton;
class UTexture2D;
class UDeliveryOrderCardWidget;

/**
 * UDeliveryAppWidget
 *
 * 배달의 민족 스타일 배달 앱 전체 화면 위젯.
 * - 헤더(주황색) + 서브헤더 + 스크롤 가능한 주문 카드 목록으로 구성.
 * - Blueprint Designer 없이 C++ 코드만으로 모든 레이아웃을 생성.
 * - UDeliveryPhoneWidget에서 OpenApp()/RefreshOrders()를 호출해 사용.
 */
UCLASS()
class GAMESET_API UDeliveryAppWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ────────────────────────────────────────────────────────────────────
	// 델리게이트 선언 (수락/거절 결과를 PhoneWidget으로 전달)
	// ────────────────────────────────────────────────────────────────────

	/** 카드에서 수락이 눌렸을 때 PhoneWidget으로 OrderID를 전달 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOrderAction, const FString&, OrderID);

	UPROPERTY(BlueprintAssignable, Category = "DeliveryApp|Events")
	FOnOrderAction OnOrderAccepted;

	/** 카드에서 거절이 눌렸을 때 PhoneWidget으로 OrderID를 전달 */
	UPROPERTY(BlueprintAssignable, Category = "DeliveryApp|Events")
	FOnOrderAction OnOrderRejected;

	// ────────────────────────────────────────────────────────────────────
	// 외관 설정
	// ────────────────────────────────────────────────────────────────────

	/** 헤더에 표시할 앱 아이콘 텍스처 (nullptr이면 생략) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Appearance")
	UTexture2D* AppHeaderIcon = nullptr;

	// ────────────────────────────────────────────────────────────────────
	// 퍼블릭 인터페이스
	// ────────────────────────────────────────────────────────────────────

	/**
	 * 주문 목록을 새로고침한다.
	 * UDeliveryPhoneWidget 또는 DeliveryManagerComponent에서 호출.
	 * @param PendingOrders  표시할 Pending 상태 주문 배열
	 */
	UFUNCTION(BlueprintCallable, Category = "DeliveryApp")
	void RefreshOrders(const TArray<FDeliveryOrder>& PendingOrders);

	/** 앱 화면을 표시 상태로 전환 */
	UFUNCTION(BlueprintCallable, Category = "DeliveryApp")
	void OpenApp();

	/** 앱 화면을 숨김 상태로 전환 */
	UFUNCTION(BlueprintCallable, Category = "DeliveryApp")
	void CloseApp();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

private:
	// ────────────────────────────────────────────────────────────────────
	// 자식 위젯 포인터 (BuildLayout에서 생성 후 저장)
	// ────────────────────────────────────────────────────────────────────

	/** 주문 카드들이 동적으로 추가되는 스크롤 영역 */
	UPROPERTY()
	UScrollBox* OrderScrollBox = nullptr;

	/** 주문이 없을 때 표시하는 안내 텍스트 */
	UPROPERTY()
	UTextBlock* EmptyText = nullptr;

	/** 닫기 버튼 */
	UPROPERTY()
	UButton* CloseButton = nullptr;

	// ────────────────────────────────────────────────────────────────────
	// 내부 함수
	// ────────────────────────────────────────────────────────────────────

	/** 전체 앱 레이아웃을 C++로 구성 (NativeConstruct에서 호출) */
	void BuildLayout();

	/** 카드의 수락 이벤트를 상위로 전달 */
	UFUNCTION()
	void OnCardAccepted(const FString& OrderID);

	/** 카드의 거절 이벤트를 상위로 전달 */
	UFUNCTION()
	void OnCardRejected(const FString& OrderID);

	/** 헤더의 닫기(✕) 버튼 클릭 콜백 */
	UFUNCTION()
	void OnCloseClicked();

	// ────────────────────────────────────────────────────────────────────
	// 색상 상수
	// ────────────────────────────────────────────────────────────────────

	/** 헤더 주황색 배경 (#FF6B35) */
	static FLinearColor OrangeColor()      { return FLinearColor(1.f, 0.42f, 0.21f, 1.f); }

	/** 서브헤더 연한 주황색 배경 */
	static FLinearColor LightOrangeColor() { return FLinearColor(1.f, 0.85f, 0.75f, 1.f); }

	/** 앱 전체 배경 밝은 회색 */
	static FLinearColor BgColor()          { return FLinearColor(0.95f, 0.95f, 0.95f, 1.f); }
};
