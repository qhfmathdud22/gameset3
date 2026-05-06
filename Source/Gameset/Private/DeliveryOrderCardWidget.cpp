#include "DeliveryOrderCardWidget.h"

// UMG 컴포넌트 헤더
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"

// UMG 위젯 트리
#include "Blueprint/WidgetTree.h"

// 슬레이트 스타일
#include "Styling/SlateTypes.h"

// ────────────────────────────────────────────────────────────────────────────────
// RebuildWidget
// UUserWidget 내부에서 Slate 위젯 트리를 재구성할 때 호출된다.
// 부모 구현을 먼저 호출해 WidgetTree가 초기화되도록 한다.
// ────────────────────────────────────────────────────────────────────────────────
TSharedRef<SWidget> UDeliveryOrderCardWidget::RebuildWidget()
{
	// 부모가 WidgetTree를 초기화하므로 반드시 먼저 호출
	TSharedRef<SWidget> Result = Super::RebuildWidget();
	return Result;
}

// ────────────────────────────────────────────────────────────────────────────────
// NativeConstruct
// 위젯이 뷰포트에 추가된 직후 호출. 여기서 레이아웃을 빌드한다.
// ────────────────────────────────────────────────────────────────────────────────
void UDeliveryOrderCardWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildLayout();
}

void UDeliveryOrderCardWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

// ────────────────────────────────────────────────────────────────────────────────
// BuildLayout
// WidgetTree->ConstructWidget<T>() 패턴으로 전체 카드 UI를 생성한다.
//
// 레이아웃 구조:
//   [Border - 흰색 배경, 패딩 12]
//     └─ [VerticalBox]
//          ├─ [HorizontalBox] 헤더 (아이콘 | 이름+설명 | 보상)
//          ├─ [HorizontalBox] 거리 + 남은 시간
//          └─ [HorizontalBox] 거절 버튼 + 수락 버튼
// ────────────────────────────────────────────────────────────────────────────────
void UDeliveryOrderCardWidget::BuildLayout()
{
	// WidgetTree가 없으면 빌드 불가 (방어 코드)
	if (!WidgetTree)
	{
		return;
	}

	// ── 루트: 흰색 Border (카드 배경) ────────────────────────────────────
	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RootBorder"));
	RootBorder->SetBrushColor(FLinearColor::White);
	RootBorder->SetPadding(FMargin(12.f));

	// WidgetTree의 루트 위젯으로 등록
	WidgetTree->RootWidget = RootBorder;

	// ── 전체 세로 박스 ────────────────────────────────────────────────────
	UVerticalBox* MainVBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainVBox"));
	RootBorder->SetContent(MainVBox);


	// ════════════════════════════════════════════════════════════════════
	// 섹션 1: 헤더 (아이콘 | 음식점 이름+물품 설명 | 보상 금액)
	// ════════════════════════════════════════════════════════════════════
	UHorizontalBox* HeaderHBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HeaderHBox"));
	{
		UVerticalBoxSlot* HeaderSlot = MainVBox->AddChildToVerticalBox(HeaderHBox);
		HeaderSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		HeaderSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	// ── 1-1: 음식점 아이콘 (40x40 SizeBox로 고정 크기) ──────────────────
	{
		USizeBox* IconSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("IconSizeBox"));
		IconSizeBox->SetWidthOverride(40.f);
		IconSizeBox->SetHeightOverride(40.f);

		RestaurantImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("RestaurantImage"));
		// 텍스처가 있으면 표시, 없으면 투명 처리
		if (RestaurantIcon)
		{
			RestaurantImage->SetBrushFromTexture(RestaurantIcon, true);
		}
		else
		{
			RestaurantImage->SetColorAndOpacity(FLinearColor::Transparent);
		}
		IconSizeBox->SetContent(RestaurantImage);

		UHorizontalBoxSlot* IconSlot = HeaderHBox->AddChildToHorizontalBox(IconSizeBox);
		IconSlot->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
		IconSlot->SetVerticalAlignment(VAlign_Center);
		IconSlot->SetHorizontalAlignment(HAlign_Left);
		IconSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}

	// ── 1-2: 음식점 이름 + 물품 설명 (세로로 쌓임) ──────────────────────
	{
		UVerticalBox* InfoVBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InfoVBox"));

		// 음식점 이름 텍스트
		RestaurantNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RestaurantNameText"));
		RestaurantNameText->SetText(FText::FromString(TEXT("음식점 이름")));
		RestaurantNameText->SetColorAndOpacity(FSlateColor(FLinearColor::Black));
		{
			FSlateFontInfo FontInfo = RestaurantNameText->GetFont();
			FontInfo.Size = 18;
			FontInfo.TypefaceFontName = FName("Bold");
			RestaurantNameText->SetFont(FontInfo);
		}
		{
			UVerticalBoxSlot* NameSlot = InfoVBox->AddChildToVerticalBox(RestaurantNameText);
			NameSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 2.f));
		}

		// 물품 설명 텍스트 (회색, 작은 크기)
		ItemDescText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ItemDescText"));
		ItemDescText->SetText(FText::FromString(TEXT("물품 설명")));
		ItemDescText->SetColorAndOpacity(FSlateColor(GrayColor()));
		{
			FSlateFontInfo FontInfo = ItemDescText->GetFont();
			FontInfo.Size = 13;
			ItemDescText->SetFont(FontInfo);
		}
		InfoVBox->AddChildToVerticalBox(ItemDescText);

		// Fill로 남은 공간 차지
		UHorizontalBoxSlot* InfoSlot = HeaderHBox->AddChildToHorizontalBox(InfoVBox);
		InfoSlot->SetVerticalAlignment(VAlign_Center);
		InfoSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	// ── 1-3: 보상 금액 (우측 정렬, 주황색 Bold) ──────────────────────────
	{
		UVerticalBox* RewardVBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RewardVBox"));

		RewardText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RewardText"));
		RewardText->SetText(FText::FromString(TEXT("₩0")));
		RewardText->SetColorAndOpacity(FSlateColor(OrangeColor()));
		RewardText->SetJustification(ETextJustify::Right);
		{
			FSlateFontInfo FontInfo = RewardText->GetFont();
			FontInfo.Size = 20;
			FontInfo.TypefaceFontName = FName("Bold");
			RewardText->SetFont(FontInfo);
		}
		{
			UVerticalBoxSlot* RewardSlot = RewardVBox->AddChildToVerticalBox(RewardText);
			RewardSlot->SetHorizontalAlignment(HAlign_Right);
		}

		UHorizontalBoxSlot* RewardBoxSlot = HeaderHBox->AddChildToHorizontalBox(RewardVBox);
		RewardBoxSlot->SetVerticalAlignment(VAlign_Center);
		RewardBoxSlot->SetHorizontalAlignment(HAlign_Right);
		RewardBoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}


	// ════════════════════════════════════════════════════════════════════
	// 섹션 2: 거리 + 남은 수락 시간
	// ════════════════════════════════════════════════════════════════════
	UHorizontalBox* InfoHBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("InfoHBox"));
	{
		UVerticalBoxSlot* InfoRowSlot = MainVBox->AddChildToVerticalBox(InfoHBox);
		InfoRowSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 10.f));
		InfoRowSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	// 거리 텍스트 (회색, 12pt)
	DistanceText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DistanceText"));
	DistanceText->SetText(FText::FromString(TEXT("📍 0.0km")));
	DistanceText->SetColorAndOpacity(FSlateColor(GrayColor()));
	{
		FSlateFontInfo FontInfo = DistanceText->GetFont();
		FontInfo.Size = 12;
		DistanceText->SetFont(FontInfo);
	}
	{
		UHorizontalBoxSlot* DistSlot = InfoHBox->AddChildToHorizontalBox(DistanceText);
		DistSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		DistSlot->SetVerticalAlignment(VAlign_Center);
	}

	// 남은 시간 텍스트 (빨간색, 12pt, 우측 정렬)
	TimeRemainingText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TimeRemainingText"));
	TimeRemainingText->SetText(FText::FromString(TEXT("⏱ 30초")));
	TimeRemainingText->SetColorAndOpacity(FSlateColor(RedColor()));
	TimeRemainingText->SetJustification(ETextJustify::Right);
	{
		FSlateFontInfo FontInfo = TimeRemainingText->GetFont();
		FontInfo.Size = 12;
		TimeRemainingText->SetFont(FontInfo);
	}
	{
		UHorizontalBoxSlot* TimeSlot = InfoHBox->AddChildToHorizontalBox(TimeRemainingText);
		TimeSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		TimeSlot->SetVerticalAlignment(VAlign_Center);
	}


	// ════════════════════════════════════════════════════════════════════
	// 섹션 3: 버튼 행 (거절 | 수락하기)
	// ════════════════════════════════════════════════════════════════════
	UHorizontalBox* BtnHBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BtnHBox"));
	{
		UVerticalBoxSlot* BtnRowSlot = MainVBox->AddChildToVerticalBox(BtnHBox);
		BtnRowSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	// ── 거절 버튼 (회색 배경, 흰색 글씨) ────────────────────────────────
	{
		RejectBtn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RejectBtn"));

		// 버튼 스타일: 회색 배경
		FButtonStyle RejectStyle = RejectBtn->GetStyle();
		RejectStyle.Normal.TintColor      = FSlateColor(DarkGrayColor());
		RejectStyle.Hovered.TintColor     = FSlateColor(FLinearColor(0.45f, 0.45f, 0.45f, 1.f));
		RejectStyle.Pressed.TintColor     = FSlateColor(FLinearColor(0.25f, 0.25f, 0.25f, 1.f));
		RejectStyle.Normal.DrawAs         = ESlateBrushDrawType::RoundedBox;
		RejectStyle.Hovered.DrawAs        = ESlateBrushDrawType::RoundedBox;
		RejectStyle.Pressed.DrawAs        = ESlateBrushDrawType::RoundedBox;
		RejectBtn->SetStyle(RejectStyle);

		// 거절 버튼 텍스트
		UTextBlock* RejectLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RejectLabel"));
		RejectLabel->SetText(FText::FromString(TEXT("거절")));
		RejectLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		{
			FSlateFontInfo FontInfo = RejectLabel->GetFont();
			FontInfo.Size = 14;
			RejectLabel->SetFont(FontInfo);
		}
		RejectBtn->SetContent(RejectLabel);

		// 클릭 이벤트 바인딩
		RejectBtn->OnClicked.AddDynamic(this, &UDeliveryOrderCardWidget::OnRejectClicked);

		// 레이아웃에 추가 (40% 너비)
		UHorizontalBoxSlot* RejectSlot = BtnHBox->AddChildToHorizontalBox(RejectBtn);
		RejectSlot->SetPadding(FMargin(0.f, 0.f, 6.f, 0.f));
		RejectSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		RejectSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	// ── 수락 버튼 (주황색 배경, 흰색 Bold 글씨) ──────────────────────────
	{
		AcceptBtn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("AcceptBtn"));

		// 버튼 스타일: 주황색 배경
		FButtonStyle AcceptStyle = AcceptBtn->GetStyle();
		AcceptStyle.Normal.TintColor      = FSlateColor(OrangeColor());
		AcceptStyle.Hovered.TintColor     = FSlateColor(FLinearColor(1.f, 0.55f, 0.35f, 1.f));
		AcceptStyle.Pressed.TintColor     = FSlateColor(FLinearColor(0.85f, 0.30f, 0.10f, 1.f));
		AcceptStyle.Normal.DrawAs         = ESlateBrushDrawType::RoundedBox;
		AcceptStyle.Hovered.DrawAs        = ESlateBrushDrawType::RoundedBox;
		AcceptStyle.Pressed.DrawAs        = ESlateBrushDrawType::RoundedBox;
		AcceptBtn->SetStyle(AcceptStyle);

		// 수락 버튼 텍스트
		UTextBlock* AcceptLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AcceptLabel"));
		AcceptLabel->SetText(FText::FromString(TEXT("수락하기")));
		AcceptLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		{
			FSlateFontInfo FontInfo = AcceptLabel->GetFont();
			FontInfo.Size = 14;
			FontInfo.TypefaceFontName = FName("Bold");
			AcceptLabel->SetFont(FontInfo);
		}
		AcceptBtn->SetContent(AcceptLabel);

		// 클릭 이벤트 바인딩
		AcceptBtn->OnClicked.AddDynamic(this, &UDeliveryOrderCardWidget::OnAcceptClicked);

		// 레이아웃에 추가 (나머지 너비 채움)
		UHorizontalBoxSlot* AcceptSlot = BtnHBox->AddChildToHorizontalBox(AcceptBtn);
		AcceptSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		AcceptSlot->SetHorizontalAlignment(HAlign_Fill);
	}
}

// ────────────────────────────────────────────────────────────────────────────────
// SetOrderData
// 주문 데이터를 받아 각 텍스트 위젯에 반영한다.
// BuildLayout() 이후 언제든지 호출 가능하며, 위젯 포인터가 nullptr이면 무시한다.
// ────────────────────────────────────────────────────────────────────────────────
void UDeliveryOrderCardWidget::SetOrderData(const FDeliveryOrder& Order)
{
	// 데이터 캐싱
	CachedOrder = Order;

	// 음식점 이름 반영
	if (RestaurantNameText)
	{
		RestaurantNameText->SetText(Order.PickupName);
	}

	// 물품 설명 반영
	if (ItemDescText)
	{
		ItemDescText->SetText(Order.ItemDescription);
	}

	// 보상 금액 반영 (₩ 기호 + 천단위 없이 정수 표시)
	if (RewardText)
	{
		RewardText->SetText(
			FText::FromString(FString::Printf(TEXT("₩%.0f"), Order.Reward))
		);
	}

	// 거리 반영 (미터 → 킬로미터 변환, 소수점 1자리)
	if (DistanceText)
	{
		DistanceText->SetText(
			FText::FromString(FString::Printf(TEXT("📍 %.1fkm"), Order.DistanceMeters / 1000.f))
		);
	}

	// 남은 수락 시간 반영 (초 단위 정수)
	if (TimeRemainingText)
	{
		TimeRemainingText->SetText(
			FText::FromString(FString::Printf(TEXT("⏱ %.0f초"), Order.PendingTimeRemaining))
		);
	}

	// 아이콘 텍스처가 이미 설정된 경우 이미지 갱신
	if (RestaurantImage && RestaurantIcon)
	{
		RestaurantImage->SetBrushFromTexture(RestaurantIcon, true);
		RestaurantImage->SetColorAndOpacity(FLinearColor::White);
	}
}

// ────────────────────────────────────────────────────────────────────────────────
// OnAcceptClicked
// 수락 버튼 클릭 시 OnAccepted 델리게이트를 브로드캐스트.
// ────────────────────────────────────────────────────────────────────────────────
void UDeliveryOrderCardWidget::OnAcceptClicked()
{
	// 캐싱된 OrderID로 델리게이트 브로드캐스트
	OnAccepted.Broadcast(CachedOrder.OrderID);
}

// ────────────────────────────────────────────────────────────────────────────────
// OnRejectClicked
// 거절 버튼 클릭 시 OnRejected 델리게이트를 브로드캐스트.
// ────────────────────────────────────────────────────────────────────────────────
void UDeliveryOrderCardWidget::OnRejectClicked()
{
	// 캐싱된 OrderID로 델리게이트 브로드캐스트
	OnRejected.Broadcast(CachedOrder.OrderID);
}
