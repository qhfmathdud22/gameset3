#include "DeliveryAppWidget.h"
#include "DeliveryOrderCardWidget.h"

// UMG 컴포넌트 헤더
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"

// UMG 위젯 트리 및 생성 헬퍼
#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"

// 슬레이트 스타일
#include "Styling/SlateTypes.h"

// ────────────────────────────────────────────────────────────────────────────────
// NativeConstruct
// 위젯이 뷰포트에 추가된 직후 호출. 레이아웃 빌드 후 기본적으로 숨김 상태.
// ────────────────────────────────────────────────────────────────────────────────
void UDeliveryAppWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// AddToViewport 전에 레이아웃 빌드 → Slate 트리에 정상 반영됨
	BuildLayout();
}

void UDeliveryAppWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 앱은 기본적으로 숨김 상태로 시작 (OpenApp() 호출 시 표시)
	SetVisibility(ESlateVisibility::Collapsed);
}

// ────────────────────────────────────────────────────────────────────────────────
// BuildLayout
// 배달 앱 전체 화면을 C++로 구성한다.
//
// 레이아웃 구조:
//   [VerticalBox - 전체 화면]
//     ├─ [Border - 헤더] 높이60, 주황색 배경
//     │    └─ [HorizontalBox] 🛵 배달 요청 제목 | ✕ 닫기 버튼
//     ├─ [Border - 서브헤더] 연한 주황 배경
//     │    └─ [TextBlock] "새로운 배달 요청이 있어요!"
//     └─ [ScrollBox] 주문 카드 목록 (동적 추가)
//          └─ EmptyText (주문 없을 때 표시)
// ────────────────────────────────────────────────────────────────────────────────
void UDeliveryAppWidget::BuildLayout()
{
	// WidgetTree가 없으면 빌드 불가 (방어 코드)
	if (!WidgetTree)
	{
		return;
	}

	// ── 루트: 전체 화면 VerticalBox ──────────────────────────────────────
	UVerticalBox* RootVBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RootVBox"));
	WidgetTree->RootWidget = RootVBox;


	// ════════════════════════════════════════════════════════════════════
	// 섹션 1: 헤더 (주황색, 높이 60)
	// ════════════════════════════════════════════════════════════════════
	USizeBox* HeaderSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("HeaderSizeBox"));
	HeaderSizeBox->SetHeightOverride(60.f);
	{
		UVerticalBoxSlot* HeaderOuterSlot = RootVBox->AddChildToVerticalBox(HeaderSizeBox);
		HeaderOuterSlot->SetHorizontalAlignment(HAlign_Fill);
		HeaderOuterSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}

	UBorder* HeaderBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HeaderBorder"));
	HeaderBorder->SetBrushColor(OrangeColor());
	HeaderBorder->SetPadding(FMargin(16.f, 0.f));
	HeaderSizeBox->SetContent(HeaderBorder);

	// 헤더 내부 HorizontalBox (타이틀 | 닫기 버튼)
	UHorizontalBox* HeaderHBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HeaderHBox"));
	HeaderBorder->SetContent(HeaderHBox);

	// ── 헤더 타이틀 텍스트 "🛵 배달 요청" ──────────────────────────────
	{
		UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
		TitleText->SetText(FText::FromString(TEXT("🛵 배달 요청")));
		TitleText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		{
			FSlateFontInfo FontInfo = TitleText->GetFont();
			FontInfo.Size = 18;
			FontInfo.TypefaceFontName = FName("Bold");
			TitleText->SetFont(FontInfo);
		}

		UHorizontalBoxSlot* TitleSlot = HeaderHBox->AddChildToHorizontalBox(TitleText);
		TitleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		TitleSlot->SetVerticalAlignment(VAlign_Center);
		TitleSlot->SetHorizontalAlignment(HAlign_Left);
	}

	// ── 닫기 버튼 "✕" ───────────────────────────────────────────────────
	{
		CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CloseButton"));

		// 닫기 버튼 스타일: 투명 배경 (헤더 색상과 동일하게 보임)
		FButtonStyle CloseStyle = CloseButton->GetStyle();
		CloseStyle.Normal.TintColor  = FSlateColor(FLinearColor::Transparent);
		CloseStyle.Hovered.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.2f));
		CloseStyle.Pressed.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.4f));
		CloseButton->SetStyle(CloseStyle);

		UTextBlock* CloseLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CloseLabel"));
		CloseLabel->SetText(FText::FromString(TEXT("✕")));
		CloseLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		{
			FSlateFontInfo FontInfo = CloseLabel->GetFont();
			FontInfo.Size = 18;
			CloseLabel->SetFont(FontInfo);
		}
		CloseButton->SetContent(CloseLabel);

		// 닫기 클릭 이벤트 바인딩
		CloseButton->OnClicked.AddDynamic(this, &UDeliveryAppWidget::OnCloseClicked);

		UHorizontalBoxSlot* CloseSlot = HeaderHBox->AddChildToHorizontalBox(CloseButton);
		CloseSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		CloseSlot->SetVerticalAlignment(VAlign_Center);
		CloseSlot->SetHorizontalAlignment(HAlign_Right);
	}


	// ════════════════════════════════════════════════════════════════════
	// 섹션 2: 서브헤더 (연한 주황 배경 + 안내 텍스트)
	// ════════════════════════════════════════════════════════════════════
	UBorder* SubHeaderBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("SubHeaderBorder"));
	SubHeaderBorder->SetBrushColor(LightOrangeColor());
	SubHeaderBorder->SetPadding(FMargin(16.f, 10.f));
	{
		UVerticalBoxSlot* SubHeaderSlot = RootVBox->AddChildToVerticalBox(SubHeaderBorder);
		SubHeaderSlot->SetHorizontalAlignment(HAlign_Fill);
		SubHeaderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}

	UTextBlock* SubHeaderText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SubHeaderText"));
	SubHeaderText->SetText(FText::FromString(TEXT("새로운 배달 요청이 있어요!")));
	SubHeaderText->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.25f, 0.f, 1.f)));
	{
		FSlateFontInfo FontInfo = SubHeaderText->GetFont();
		FontInfo.Size = 14;
		SubHeaderText->SetFont(FontInfo);
	}
	SubHeaderBorder->SetContent(SubHeaderText);


	// ════════════════════════════════════════════════════════════════════
	// 섹션 3: 스크롤 가능한 주문 카드 목록
	// ════════════════════════════════════════════════════════════════════
	UBorder* ScrollBgBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ScrollBgBorder"));
	ScrollBgBorder->SetBrushColor(BgColor());
	ScrollBgBorder->SetPadding(FMargin(0.f));
	{
		UVerticalBoxSlot* ScrollBgSlot = RootVBox->AddChildToVerticalBox(ScrollBgBorder);
		ScrollBgSlot->SetHorizontalAlignment(HAlign_Fill);
		ScrollBgSlot->SetVerticalAlignment(VAlign_Fill);
		// Fill: 남은 수직 공간 전부 차지
		ScrollBgSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	// ScrollBox: 주문 카드들이 동적으로 추가되는 영역
	OrderScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("OrderScrollBox"));
	OrderScrollBox->SetOrientation(Orient_Vertical);
	OrderScrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);
	ScrollBgBorder->SetContent(OrderScrollBox);

	// 빈 상태 안내 텍스트 (주문이 없을 때 표시)
	EmptyText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("EmptyText"));
	EmptyText->SetText(FText::FromString(TEXT("대기 중인 주문이 없습니다")));
	EmptyText->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 1.f)));
	EmptyText->SetJustification(ETextJustify::Center);
	{
		FSlateFontInfo FontInfo = EmptyText->GetFont();
		FontInfo.Size = 16;
		EmptyText->SetFont(FontInfo);
	}
	{
		UScrollBoxSlot* EmptySlot = Cast<UScrollBoxSlot>(OrderScrollBox->AddChild(EmptyText));
		if (EmptySlot)
		{
			EmptySlot->SetPadding(FMargin(0.f, 40.f));
			EmptySlot->SetHorizontalAlignment(HAlign_Center);
		}
	}
}

// ────────────────────────────────────────────────────────────────────────────────
// RefreshOrders
// Pending 주문 배열을 받아 ScrollBox 안의 카드를 전부 재구성한다.
// 카드가 0개이면 EmptyText를 표시한다.
// ────────────────────────────────────────────────────────────────────────────────
void UDeliveryAppWidget::RefreshOrders(const TArray<FDeliveryOrder>& PendingOrders)
{
	// ScrollBox가 아직 생성되지 않았으면 무시 (방어 코드)
	if (!OrderScrollBox)
	{
		return;
	}

	// 기존 카드 전부 제거
	OrderScrollBox->ClearChildren();

	// 주문이 없으면 안내 텍스트만 표시
	if (PendingOrders.Num() == 0)
	{
		if (EmptyText)
		{
			UScrollBoxSlot* EmptySlot = Cast<UScrollBoxSlot>(OrderScrollBox->AddChild(EmptyText));
			if (EmptySlot)
			{
				EmptySlot->SetPadding(FMargin(0.f, 40.f));
				EmptySlot->SetHorizontalAlignment(HAlign_Center);
			}
		}
		return;
	}

	// 주문마다 카드 위젯을 생성해 ScrollBox에 추가
	for (const FDeliveryOrder& Order : PendingOrders)
	{
		// CreateWidget: PlayerController 컨텍스트로 카드 생성
		UDeliveryOrderCardWidget* Card = CreateWidget<UDeliveryOrderCardWidget>(
			GetOwningPlayer(),
			UDeliveryOrderCardWidget::StaticClass()
		);

		if (!Card)
		{
			continue;
		}

		// 주문 데이터 세팅
		Card->SetOrderData(Order);

		// 수락/거절 델리게이트 바인딩 (카드 → AppWidget → PhoneWidget)
		Card->OnAccepted.AddDynamic(this, &UDeliveryAppWidget::OnCardAccepted);
		Card->OnRejected.AddDynamic(this, &UDeliveryAppWidget::OnCardRejected);

		// ScrollBox에 카드 추가
		UScrollBoxSlot* CardSlot = Cast<UScrollBoxSlot>(OrderScrollBox->AddChild(Card));
		if (CardSlot)
		{
			// 카드 사이에 여백 (아래 8px)
			CardSlot->SetPadding(FMargin(12.f, 8.f, 12.f, 0.f));
			CardSlot->SetHorizontalAlignment(HAlign_Fill);
		}
	}
}

// ────────────────────────────────────────────────────────────────────────────────
// OpenApp / CloseApp
// 앱 화면의 가시성을 전환한다.
// ────────────────────────────────────────────────────────────────────────────────
void UDeliveryAppWidget::OpenApp()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UDeliveryAppWidget::CloseApp()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

// ────────────────────────────────────────────────────────────────────────────────
// 델리게이트 콜백
// ────────────────────────────────────────────────────────────────────────────────

void UDeliveryAppWidget::OnCardAccepted(const FString& OrderID)
{
	// 카드의 수락 이벤트를 PhoneWidget으로 전달
	OnOrderAccepted.Broadcast(OrderID);
}

void UDeliveryAppWidget::OnCardRejected(const FString& OrderID)
{
	// 카드의 거절 이벤트를 PhoneWidget으로 전달
	OnOrderRejected.Broadcast(OrderID);
}

void UDeliveryAppWidget::OnCloseClicked()
{
	// 닫기 버튼 클릭 시 앱 화면 숨김
	CloseApp();
}
