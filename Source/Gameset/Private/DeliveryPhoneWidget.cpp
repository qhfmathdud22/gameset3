#include "DeliveryPhoneWidget.h"
#include "DeliveryAppWidget.h"
#include "DeliveryNotifBannerWidget.h"
#include "DeliveryComponent.h"
#include "DeliveryManagerComponent.h"
#include "GamesetGameMode.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Styling/SlateTypes.h"

// ── DeliveryComponent 참조 ────────────────────────────────────────────────────
// 게임 시간 표시에 사용 (플레이어 Pawn에 붙어있는 컴포넌트)

UDeliveryComponent* UDeliveryPhoneWidget::GetDC() const
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return nullptr;
	APawn* Pawn = PC->GetPawn();
	return Pawn ? Pawn->FindComponentByClass<UDeliveryComponent>() : nullptr;
}

// ── DeliveryManagerComponent 참조 ────────────────────────────────────────────

UDeliveryManagerComponent* UDeliveryPhoneWidget::FindDeliveryManager() const
{
	AGameModeBase* GM = UGameplayStatics::GetGameMode(GetWorld());
	if (!GM) return nullptr;
	return GM->FindComponentByClass<UDeliveryManagerComponent>();
}

// ── NativeConstruct ───────────────────────────────────────────────────────────
// 위젯 생성 시 한 번 실행.
// DeliveryManager 캐싱 → 델리게이트 바인딩 → 오버레이 위젯 뷰포트 직접 추가

void UDeliveryPhoneWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// DeliveryManagerComponent 캐싱
	CachedDeliveryManager = FindDeliveryManager();

	// ── 오버레이 위젯 먼저 생성 ──────────────────────────────────────
	// 반드시 델리게이트 바인딩보다 먼저 생성해야
	// HandleNewOrderPending 내부에서 NotifBannerOverlay가 null이 아님이 보장됨
	BuildNotificationOverlay();
	BuildDeliveryAppWidget();

	// ── DeliveryManager 바인딩 ────────────────────────────────────────
	if (CachedDeliveryManager)
	{
		CachedDeliveryManager->OnNewOrderPending.AddDynamic(
			this, &UDeliveryPhoneWidget::HandleNewOrderPending);

		// 바인딩 직후 이미 Pending된 오더가 있으면 즉시 알림 표시
		const TArray<FDeliveryOrder>& Existing = CachedDeliveryManager->GetPendingOrders();
		if (Existing.Num() > 0)
		{
			HandleNewOrderPending(Existing[0]);
		}
	}
}

// ── NativeDestruct ────────────────────────────────────────────────────────────
// 폰 위젯이 제거될 때 뷰포트에 직접 추가한 오버레이도 함께 제거

void UDeliveryPhoneWidget::NativeDestruct()
{
	// 알림 배너는 항상 뷰포트에 직접 추가됨 → 제거
	if (NotifBannerOverlay && NotifBannerOverlay->IsInViewport())
	{
		NotifBannerOverlay->RemoveFromParent();
	}

	// 배달 앱은 PhoneAppContainer(캔버스)에 있으면 자동 정리됨.
	// 폴백으로 뷰포트에 추가된 경우에만 직접 제거.
	if (DeliveryAppWidgetInstance && !PhoneAppContainer
		&& DeliveryAppWidgetInstance->IsInViewport())
	{
		DeliveryAppWidgetInstance->RemoveFromParent();
	}

	Super::NativeDestruct();
}

// ── 알림 배너 오버레이 빌드 ──────────────────────────────────────────────────
// DeliveryNotifBannerWidget을 뷰포트에 직접 추가 (ZOrder=200).
// PhoneWidget 캔버스 안에 넣지 않으므로 폰 열림/닫힘과 완전히 독립된다.

void UDeliveryPhoneWidget::BuildNotificationOverlay()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	NotifBannerOverlay = CreateWidget<UDeliveryNotifBannerWidget>(
		PC, UDeliveryNotifBannerWidget::StaticClass());
	if (!NotifBannerOverlay) return;

	// 배너 클릭 시 배달 앱 열기
	NotifBannerOverlay->OnBannerClicked.AddDynamic(
		this, &UDeliveryPhoneWidget::OnNotificationClicked);

	// 뷰포트에 ZOrder=200으로 추가 (게임 HUD보다 위, 항상 최상단)
	NotifBannerOverlay->AddToViewport(200);
}

// ── 배달 앱 위젯 빌드 ────────────────────────────────────────────────────────
// PhoneAppContainer(CanvasPanel)가 있으면 폰 화면 안에 배달 앱을 추가한다.
// 없으면 전체화면 뷰포트 오버레이(ZOrder=100)로 폴백.
//
// [BP 설정 방법]
//   BP 디자이너에서 CanvasPanel 하나를 폰 화면 영역(이미지) 위에 배치하고
//   이름을 정확히 "PhoneAppContainer" 로 변경하면 자동으로 연결됩니다.

void UDeliveryPhoneWidget::BuildDeliveryAppWidget()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	// 사용할 클래스: BP에서 지정한 서브클래스 우선, 없으면 기본 클래스
	TSubclassOf<UDeliveryAppWidget> ClassToUse = DeliveryAppWidgetClass
		? DeliveryAppWidgetClass
		: TSubclassOf<UDeliveryAppWidget>(UDeliveryAppWidget::StaticClass());

	DeliveryAppWidgetInstance = CreateWidget<UDeliveryAppWidget>(PC, ClassToUse);
	if (!DeliveryAppWidgetInstance) return;

	// 수락/거절 델리게이트 바인딩
	DeliveryAppWidgetInstance->OnOrderAccepted.AddDynamic(
		this, &UDeliveryPhoneWidget::HandleAppOrderAccepted);
	DeliveryAppWidgetInstance->OnOrderRejected.AddDynamic(
		this, &UDeliveryPhoneWidget::HandleAppOrderRejected);

	// 앱 아이콘 텍스처 전달
	if (DeliveryAppIconTexture)
	{
		DeliveryAppWidgetInstance->AppHeaderIcon = DeliveryAppIconTexture;
	}

	if (PhoneAppContainer)
	{
		// ── 폰 캔버스 안에 배달 앱 추가 (휴대폰 화면 내부에 표시) ──────
		UCanvasPanelSlot* AppSlot = Cast<UCanvasPanelSlot>(
			PhoneAppContainer->AddChild(DeliveryAppWidgetInstance));
		if (AppSlot)
		{
			// 앵커: 캔버스 전체를 꽉 채움 (0,0 → 1,1)
			AppSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
			// 오프셋 0 → 캔버스 크기와 동일
			AppSlot->SetOffsets(FMargin(0.f));
			// ZOrder: 폰 배경 위, 알림 배너(뷰포트 200) 아래
			AppSlot->SetZOrder(50);
		}
		UE_LOG(LogTemp, Log, TEXT("DeliveryApp → PhoneAppContainer 안에 배치됨"));
	}
	else
	{
		// ── 폴백: 전체화면 뷰포트 오버레이 ─────────────────────────────
		// PhoneAppContainer 가 BP에 없을 때 이전 동작 유지
		DeliveryAppWidgetInstance->AddToViewport(100);
		UE_LOG(LogTemp, Warning,
			TEXT("PhoneAppContainer 없음 → 전체화면 뷰포트 폴백 (BP에 CanvasPanel 'PhoneAppContainer' 추가 권장)"));
	}

	// 처음에는 숨김 (OpenDeliveryApp() 호출 시 표시)
	DeliveryAppWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
}

// ── HandleNewOrderPending ─────────────────────────────────────────────────────
// 새 오더가 Pending 상태로 생성될 때 자동 호출.

void UDeliveryPhoneWidget::HandleNewOrderPending(const FDeliveryOrder& Order)
{
	UE_LOG(LogTemp, Warning, TEXT("★★★ HandleNewOrderPending 호출됨! NotifBannerOverlay=%s ★★★"),
		NotifBannerOverlay ? TEXT("존재") : TEXT("NULL"));

	// 1. 알림 효과음 재생 (BP 클래스 디폴트에서 Sound 할당 필요)
	if (NotificationSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), NotificationSound);
	}

	// 2. 알림 배너 오버레이 표시 (폰 상태와 무관하게 항상 뜸)
	if (NotifBannerOverlay)
	{
		UTexture2D* Icon = NotificationIconTexture ? NotificationIconTexture : DeliveryAppIconTexture;
		NotifBannerOverlay->ShowBanner(Order.ItemDescription, Icon, AppPrimaryColor);
	}

	// 3. 폰 잠금화면 알림 배지 표시
	//    BP에서 "PhoneNotifBadge" / "PhoneNotifText" 이름의 위젯이 있으면 자동 표시
	if (PhoneNotifBadge)
	{
		// 주황색 배경으로 배지 강조
		PhoneNotifBadge->SetBrushColor(AppPrimaryColor);
		PhoneNotifBadge->SetVisibility(ESlateVisibility::Visible);
	}
	if (PhoneNotifText)
	{
		FString NotifStr = FString::Printf(TEXT("새 배달 요청  %s"),
			*Order.ItemDescription.ToString());
		PhoneNotifText->SetText(FText::FromString(NotifStr));
		PhoneNotifText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	}

	// 4. BP 이벤트 트리거 (추가 애니메이션/뱃지 구현 가능)
	OnNewOrderNotification(Order);

	// 5. 주문 목록 UI 갱신
	RefreshOrderList();
}

// ── 알림 배너 클릭 → 배달 앱 열기 ──────────────────────────────────────────

void UDeliveryPhoneWidget::OnNotificationClicked()
{
	// 배너는 이미 HideBanner()가 호출됐지만 혹시 모를 경우 대비
	if (NotifBannerOverlay)
		NotifBannerOverlay->HideBanner();

	OpenDeliveryApp();
}

// ── OpenDeliveryApp / CloseDeliveryApp ────────────────────────────────────────

void UDeliveryPhoneWidget::OpenDeliveryApp()
{
	if (!DeliveryAppWidgetInstance) return;

	// 최신 Pending 주문 목록으로 갱신
	if (CachedDeliveryManager)
	{
		DeliveryAppWidgetInstance->RefreshOrders(CachedDeliveryManager->GetPendingOrders());
	}

	DeliveryAppWidgetInstance->SetVisibility(ESlateVisibility::Visible);

	// 앱을 열었으므로 잠금화면 알림 배지 숨김
	if (PhoneNotifBadge)
	{
		PhoneNotifBadge->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UDeliveryPhoneWidget::CloseDeliveryApp()
{
	if (DeliveryAppWidgetInstance)
	{
		DeliveryAppWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// ── 앱 위젯 수락/거절 핸들러 ─────────────────────────────────────────────────

void UDeliveryPhoneWidget::HandleAppOrderAccepted(const FString& OrderID)
{
	AcceptDeliveryOrder(OrderID);

	if (DeliveryAppWidgetInstance && CachedDeliveryManager)
	{
		DeliveryAppWidgetInstance->RefreshOrders(CachedDeliveryManager->GetPendingOrders());
	}
}

void UDeliveryPhoneWidget::HandleAppOrderRejected(const FString& OrderID)
{
	RejectDeliveryOrder(OrderID);

	if (DeliveryAppWidgetInstance && CachedDeliveryManager)
	{
		DeliveryAppWidgetInstance->RefreshOrders(CachedDeliveryManager->GetPendingOrders());
	}
}

// ── 실제 시간 ─────────────────────────────────────────────────────────────────

FText UDeliveryPhoneWidget::GetRealTimeText() const
{
	FDateTime Now   = FDateTime::Now();
	int32 Hour      = Now.GetHour();
	int32 Minute    = Now.GetMinute();
	const TCHAR* AP = (Hour < 12) ? TEXT("오전") : TEXT("오후");
	int32 H12       = Hour % 12;
	if (H12 == 0) H12 = 12;
	return FText::FromString(FString::Printf(TEXT("%s %d:%02d"), AP, H12, Minute));
}

FText UDeliveryPhoneWidget::GetRealDateText() const
{
	FDateTime Now = FDateTime::Now();
	const TCHAR* Days[] = { TEXT("일"), TEXT("월"), TEXT("화"), TEXT("수"), TEXT("목"), TEXT("금"), TEXT("토") };
	return FText::FromString(FString::Printf(TEXT("%d월 %d일 %s요일"),
		Now.GetMonth(), Now.GetDay(), Days[(int32)Now.GetDayOfWeek()]));
}

// ── 게임 시간 ─────────────────────────────────────────────────────────────────

FText UDeliveryPhoneWidget::GetGameTimeText() const
{
	UDeliveryComponent* DC = GetDC();
	if (!DC) return FText::FromString(TEXT("--:--"));

	float Frac     = DC->GameTimeSeconds / UDeliveryComponent::GameDayDuration;
	float GameHour = Frac * 24.f;

	int32 Hour   = (int32)GameHour % 24;
	int32 Minute = (int32)((GameHour - (int32)GameHour) * 60.f);

	const TCHAR* AP = (Hour < 12) ? TEXT("오전") : TEXT("오후");
	int32 H12       = Hour % 12;
	if (H12 == 0) H12 = 12;

	return FText::FromString(FString::Printf(TEXT("%s %d:%02d"), AP, H12, Minute));
}

FText UDeliveryPhoneWidget::GetGameDateText() const
{
	UDeliveryComponent* DC = GetDC();
	if (!DC) return FText::FromString(TEXT("----년 --월 --일"));

	FDateTime StartDate(2026, 4, 19);
	FDateTime Current = StartDate + FTimespan::FromDays(DC->GameDay);

	const TCHAR* Days[] = { TEXT("일"), TEXT("월"), TEXT("화"), TEXT("수"), TEXT("목"), TEXT("금"), TEXT("토") };
	return FText::FromString(FString::Printf(TEXT("%d년 %d월 %d일 %s요일"),
		Current.GetYear(), Current.GetMonth(), Current.GetDay(),
		Days[(int32)Current.GetDayOfWeek()]));
}

// ── 배달 앱 데이터 함수들 ─────────────────────────────────────────────────────

TArray<FDeliveryOrder> UDeliveryPhoneWidget::GetPendingOrders() const
{
	if (!CachedDeliveryManager) return TArray<FDeliveryOrder>();
	return TArray<FDeliveryOrder>(CachedDeliveryManager->GetPendingOrders());
}

bool UDeliveryPhoneWidget::AcceptDeliveryOrder(const FString& OrderID)
{
	if (!CachedDeliveryManager) return false;
	bool bSuccess = CachedDeliveryManager->AcceptOrder(OrderID);
	if (bSuccess) RefreshOrderList();
	return bSuccess;
}

bool UDeliveryPhoneWidget::RejectDeliveryOrder(const FString& OrderID)
{
	if (!CachedDeliveryManager) return false;
	bool bSuccess = CachedDeliveryManager->RejectOrder(OrderID);
	if (bSuccess) RefreshOrderList();
	return bSuccess;
}

TArray<FDeliveryOrder> UDeliveryPhoneWidget::GetActiveOrders() const
{
	TArray<FDeliveryOrder> Result;
	if (!CachedDeliveryManager) return Result;

	for (const FDeliveryOrder& Order : CachedDeliveryManager->GetActiveOrders())
	{
		if (Order.Status == EOrderStatus::Available
			|| Order.Status == EOrderStatus::PickedUp)
		{
			Result.Add(Order);
		}
	}
	return Result;
}

// ── 포맷 유틸리티 ─────────────────────────────────────────────────────────────

FText UDeliveryPhoneWidget::GetFormattedReward(float Amount) const
{
	return FText::FromString(
		FString::Printf(TEXT("₩%s"), *FText::AsNumber((int32)Amount).ToString()));
}

FText UDeliveryPhoneWidget::GetFormattedDistance(float Meters) const
{
	if (Meters >= 1000.f)
		return FText::FromString(FString::Printf(TEXT("%.1fkm"), Meters / 1000.f));
	else
		return FText::FromString(FString::Printf(TEXT("%.0fm"), Meters));
}

FText UDeliveryPhoneWidget::GetFormattedTime(float Seconds) const
{
	int32 Min = (int32)Seconds / 60;
	int32 Sec = (int32)Seconds % 60;
	if (Min > 0)
		return FText::FromString(FString::Printf(TEXT("%d분 %d초"), Min, Sec));
	else
		return FText::FromString(FString::Printf(TEXT("%d초"), Sec));
}
