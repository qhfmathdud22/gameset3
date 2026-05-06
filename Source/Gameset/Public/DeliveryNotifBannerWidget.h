#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeliveryNotifBannerWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UCanvasPanel;

/**
 * UDeliveryNotifBannerWidget
 *
 * 게임 화면 최상단에 독립적으로 오버레이되는 알림 배너.
 * PhoneWidget 캔버스 안이 아니라 AddToViewport(200)로 직접 붙어
 * 휴대폰 상태(열림/닫힘)와 무관하게 항상 표시된다.
 *
 * 사용법:
 *   CreateWidget<UDeliveryNotifBannerWidget>(PC, ...)
 *   ->AddToViewport(200)
 *   ->ShowBanner(Description, Icon, Color)
 */
UCLASS()
class GAMESET_API UDeliveryNotifBannerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 배너 클릭 시 브로드캐스트 → PhoneWidget이 수신해 앱을 열도록 연결
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBannerClicked);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBannerClicked OnBannerClicked;

	/**
	 * 배너를 표시한다. 5초 후 자동 숨김.
	 * @param Description  주문 내용 텍스트 (예: "치킨 2마리 + 콜라")
	 * @param Icon         알림 아이콘 텍스처 (nullptr이면 아이콘 생략)
	 * @param AccentColor  타이틀 강조 색상 (배달의민족 주황 기본값)
	 */
	void ShowBanner(const FText& Description,
	                UTexture2D* Icon        = nullptr,
	                FLinearColor AccentColor = FLinearColor(1.f, 0.42f, 0.21f, 1.f));

	/** 배너를 즉시 숨긴다 (타이머도 취소) */
	void HideBanner();

protected:
	// NativeOnInitialized: CreateWidget 시점에 호출 → AddToViewport 전에 RootWidget 설정
	// NativeConstruct 대신 여기서 BuildLayout을 호출해야 Slate 트리에 제대로 반영됨
	virtual void NativeOnInitialized() override;

private:
	/** 클릭 가능한 배너 버튼 (전체 배너 영역) */
	UPROPERTY() UButton*    BannerButton = nullptr;
	/** 주문 내용 텍스트 */
	UPROPERTY() UTextBlock* DescText     = nullptr;
	/** 앱 아이콘 이미지 */
	UPROPERTY() UImage*     IconImage    = nullptr;

	/** 자동 숨김 타이머 */
	FTimerHandle AutoHideTimer;

	/** 버튼 클릭 콜백 — 배너 숨기고 OnBannerClicked 브로드캐스트 */
	UFUNCTION()
	void OnButtonClicked();

	/** 레이아웃 전체 C++ 구성 */
	void BuildLayout();
};
