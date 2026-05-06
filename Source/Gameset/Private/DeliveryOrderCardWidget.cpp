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
// 폰 화면 너비에 최적화된 카드 UI를 구성한다.
//
// 레이아웃 구조:
//   [Border - 흰색 배경, 패딩 10]
//     └─ [VerticalBox]
//          ├─ [HorizontalBox] 아이콘(32) | 음식점 이름+설명(Fill)
//          ├─ [Border - 주황색] 보상 금액 (전체 너비, 우측 정렬)
//          ├─ [HorizontalBox] 거리 | 남은 시간
//          ├─ [Border - 구분선]
//          └─ [HorizontalBox] 거절 버튼 | 수락 버튼
// ────────────────────────────────────────────────────────────────────────────────
void UDeliveryOrderCardWidget::BuildLayout()
{
	if (!WidgetTree) return;

	// ── 루트: 흰색 Border (카드 배경, 둥근 느낌의 패딩) ─────────────────
	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RootBorder"));
	RootBorder->SetBrushColor(FLinearColor::White);
	RootBorder->SetPadding(FMargin(10.f, 10.f, 10.f, 10.f));
	WidgetTree->RootWidget = RootBorder;

	UVerticalBox* MainVBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainVBox"));
	RootBorder->SetContent(MainVBox);


	// ════════════════════════════════════════════════════════════════════
	// 섹션 1: 아이콘 + 음식점 이름 + 물품 설명 (한 행)
	// ════════════════════════════════════════════════════════════════════
	UHorizontalBox* NameRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("NameRow"));
	{
		UVerticalBoxSlot* S = MainVBox->AddChildToVerticalBox(NameRow);
		S->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
		S->SetHorizontalAlignment(HAlign_Fill);
	}

	// 아이콘 (32x32)
	{
		USizeBox* IconBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("IconBox"));
		IconBox->SetWidthOverride(32.f);
		IconBox->SetHeightOverride(32.f);

		RestaurantImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("RestaurantImage"));
		if (RestaurantIcon) RestaurantImage->SetBrushFromTexture(RestaurantIcon, true);
		else                RestaurantImage->SetColorAndOpacity(FLinearColor::Transparent);
		IconBox->SetContent(RestaurantImage);

		UHorizontalBoxSlot* IS = NameRow->AddChildToHorizontalBox(IconBox);
		IS->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
		IS->SetVerticalAlignment(VAlign_Center);
		IS->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}

	// 음식점 이름 + 물품 설명 (Fill, AutoWrap)
	{
		UVerticalBox* InfoVBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InfoVBox"));

		RestaurantNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RestaurantNameText"));
		RestaurantNameText->SetText(FText::FromString(TEXT("음식점 이름")));
		RestaurantNameText->SetColorAndOpacity(FSlateColor(FLinearColor::Black));
		RestaurantNameText->SetAutoWrapText(true);   // ← 이름이 길어도 줄바꿈
		{
			FSlateFontInfo F = RestaurantNameText->GetFont();
			F.Size = 14;
			F.TypefaceFontName = FName("Bold");
			RestaurantNameText->SetFont(F);
		}
		{
			UVerticalBoxSlot* S = InfoVBox->AddChildToVerticalBox(RestaurantNameText);
			S->SetPadding(FMargin(0.f, 0.f, 0.f, 2.f));
		}

		ItemDescText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ItemDescText"));
		ItemDescText->SetText(FText::FromString(TEXT("물품 설명")));
		ItemDescText->SetColorAndOpacity(FSlateColor(GrayColor()));
		ItemDescText->SetAutoWrapText(true);         // ← 설명도 줄바꿈
		{
			FSlateFontInfo F = ItemDescText->GetFont();
			F.Size = 11;
			ItemDescText->SetFont(F);
		}
		InfoVBox->AddChildToVerticalBox(ItemDescText);

		UHorizontalBoxSlot* IS = NameRow->AddChildToHorizontalBox(InfoVBox);
		IS->SetVerticalAlignment(VAlign_Center);
		IS->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}


	// ════════════════════════════════════════════════════════════════════
	// 섹션 2: 보상 금액 (주황색 배경 배지, 전체 너비)
	// ════════════════════════════════════════════════════════════════════
	UBorder* RewardBadge = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RewardBadge"));
	RewardBadge->SetBrushColor(FLinearColor(1.f, 0.95f, 0.88f, 1.f));  // 연주황 배경
	RewardBadge->SetPadding(FMargin(8.f, 5.f));
	{
		UVerticalBoxSlot* S = MainVBox->AddChildToVerticalBox(RewardBadge);
		S->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
		S->SetHorizontalAlignment(HAlign_Fill);
	}

	RewardText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RewardText"));
	RewardText->SetText(FText::FromString(TEXT("₩0")));
	RewardText->SetColorAndOpacity(FSlateColor(OrangeColor()));
	RewardText->SetJustification(ETextJustify::Right);
	{
		FSlateFontInfo F = RewardText->GetFont();
		F.Size = 16;
		F.TypefaceFontName = FName("Bold");
		RewardText->SetFont(F);
	}
	RewardBadge->SetContent(RewardText);


	// ════════════════════════════════════════════════════════════════════
	// 섹션 3: 거리 + 남은 수락 시간 (한 행)
	// ════════════════════════════════════════════════════════════════════
	UHorizontalBox* InfoHBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("InfoHBox"));
	{
		UVerticalBoxSlot* S = MainVBox->AddChildToVerticalBox(InfoHBox);
		S->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		S->SetHorizontalAlignment(HAlign_Fill);
	}

	// 거리 (회색, 11pt)
	DistanceText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DistanceText"));
	DistanceText->SetText(FText::FromString(TEXT("0.0km")));
	DistanceText->SetColorAndOpacity(FSlateColor(GrayColor()));
	{
		FSlateFontInfo F = DistanceText->GetFont();
		F.Size = 11;
		DistanceText->SetFont(F);
	}
	{
		UHorizontalBoxSlot* S = InfoHBox->AddChildToHorizontalBox(DistanceText);
		S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		S->SetVerticalAlignment(VAlign_Center);
	}

	// 남은 시간 (빨간색, 11pt, 우측)
	TimeRemainingText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TimeRemainingText"));
	TimeRemainingText->SetText(FText::FromString(TEXT("30초")));
	TimeRemainingText->SetColorAndOpacity(FSlateColor(RedColor()));
	TimeRemainingText->SetJustification(ETextJustify::Right);
	{
		FSlateFontInfo F = TimeRemainingText->GetFont();
		F.Size = 11;
		TimeRemainingText->SetFont(F);
	}
	{
		UHorizontalBoxSlot* S = InfoHBox->AddChildToHorizontalBox(TimeRemainingText);
		S->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		S->SetVerticalAlignment(VAlign_Center);
	}


	// ════════════════════════════════════════════════════════════════════
	// 섹션 4: 구분선
	// ════════════════════════════════════════════════════════════════════
	{
		UBorder* Divider = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Divider"));
		Divider->SetBrushColor(FLinearColor(0.85f, 0.85f, 0.85f, 1.f));
		Divider->SetPadding(FMargin(0.f));
		USizeBox* DivSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("DivSizeBox"));
		DivSizeBox->SetHeightOverride(1.f);
		DivSizeBox->SetContent(Divider);

		UVerticalBoxSlot* S = MainVBox->AddChildToVerticalBox(DivSizeBox);
		S->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		S->SetHorizontalAlignment(HAlign_Fill);
	}


	// ════════════════════════════════════════════════════════════════════
	// 섹션 5: 버튼 행 (거절 | 수락하기)
	// ════════════════════════════════════════════════════════════════════
	UHorizontalBox* BtnHBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BtnHBox"));
	{
		UVerticalBoxSlot* S = MainVBox->AddChildToVerticalBox(BtnHBox);
		S->SetHorizontalAlignment(HAlign_Fill);
	}

	// 거절 버튼
	{
		RejectBtn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RejectBtn"));
		FButtonStyle Style = RejectBtn->GetStyle();
		Style.Normal.TintColor  = FSlateColor(DarkGrayColor());
		Style.Hovered.TintColor = FSlateColor(FLinearColor(0.45f, 0.45f, 0.45f, 1.f));
		Style.Pressed.TintColor = FSlateColor(FLinearColor(0.25f, 0.25f, 0.25f, 1.f));
		Style.Normal.DrawAs = Style.Hovered.DrawAs = Style.Pressed.DrawAs = ESlateBrushDrawType::RoundedBox;
		RejectBtn->SetStyle(Style);

		UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RejectLabel"));
		Label->SetText(FText::FromString(TEXT("거절")));
		Label->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		{
			FSlateFontInfo F = Label->GetFont(); F.Size = 13; Label->SetFont(F);
		}
		RejectBtn->SetContent(Label);
		RejectBtn->OnClicked.AddDynamic(this, &UDeliveryOrderCardWidget::OnRejectClicked);

		UHorizontalBoxSlot* S = BtnHBox->AddChildToHorizontalBox(RejectBtn);
		S->SetPadding(FMargin(0.f, 0.f, 5.f, 0.f));
		S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		S->SetHorizontalAlignment(HAlign_Fill);
	}

	// 수락 버튼
	{
		AcceptBtn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("AcceptBtn"));
		FButtonStyle Style = AcceptBtn->GetStyle();
		Style.Normal.TintColor  = FSlateColor(OrangeColor());
		Style.Hovered.TintColor = FSlateColor(FLinearColor(1.f, 0.55f, 0.35f, 1.f));
		Style.Pressed.TintColor = FSlateColor(FLinearColor(0.85f, 0.30f, 0.10f, 1.f));
		Style.Normal.DrawAs = Style.Hovered.DrawAs = Style.Pressed.DrawAs = ESlateBrushDrawType::RoundedBox;
		AcceptBtn->SetStyle(Style);

		UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AcceptLabel"));
		Label->SetText(FText::FromString(TEXT("수락하기")));
		Label->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		{
			FSlateFontInfo F = Label->GetFont();
			F.Size = 13;
			F.TypefaceFontName = FName("Bold");
			Label->SetFont(F);
		}
		AcceptBtn->SetContent(Label);
		AcceptBtn->OnClicked.AddDynamic(this, &UDeliveryOrderCardWidget::OnAcceptClicked);

		UHorizontalBoxSlot* S = BtnHBox->AddChildToHorizontalBox(AcceptBtn);
		S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		S->SetHorizontalAlignment(HAlign_Fill);
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
		if (Order.DistanceMeters >= 1000.f)
			DistanceText->SetText(FText::FromString(
				FString::Printf(TEXT("%.1fkm"), Order.DistanceMeters / 1000.f)));
		else
			DistanceText->SetText(FText::FromString(
				FString::Printf(TEXT("%.0fm"), Order.DistanceMeters)));
	}

	// 남은 수락 시간 반영
	if (TimeRemainingText)
	{
		const int32 Sec = FMath::CeilToInt(Order.PendingTimeRemaining);
		if (Sec >= 60)
			TimeRemainingText->SetText(FText::FromString(
				FString::Printf(TEXT("%d분 %d초"), Sec / 60, Sec % 60)));
		else
			TimeRemainingText->SetText(FText::FromString(
				FString::Printf(TEXT("%d초"), Sec)));
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
