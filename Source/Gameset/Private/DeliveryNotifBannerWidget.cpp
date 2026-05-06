#include "DeliveryNotifBannerWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Image.h"
#include "Styling/SlateTypes.h"

// ────────────────────────────────────────────────────────────────────────────────
// NativeConstruct
// ────────────────────────────────────────────────────────────────────────────────
void UDeliveryNotifBannerWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	// AddToViewport 전에 호출되므로 여기서 RootWidget을 설정하면
	// Slate 위젯 트리 빌드 시 정상 반영된다.
	BuildLayout();
}

// ────────────────────────────────────────────────────────────────────────────────
// BuildLayout
//
// 레이아웃 구조:
//   [CanvasPanel - 전체화면, SelfHitTestInvisible (게임 입력 통과)]
//     └─ [Button - 상단 중앙, 360×90, 어두운 반투명]  ← 클릭 가능
//           └─ [HorizontalBox]
//                ├─ [Image]   아이콘 38×38
//                └─ [VerticalBox]
//                     ├─ [TextBlock] "🛵 새 배달 요청" (주황)
//                     ├─ [TextBlock] 주문 설명 (흰색)
//                     └─ [TextBlock] "탭하여 확인" (회색, 소자)
// ────────────────────────────────────────────────────────────────────────────────
void UDeliveryNotifBannerWidget::BuildLayout()
{
	if (!WidgetTree) return;

	// ── 루트 캔버스: 전체화면 투명 오버레이 ─────────────────────────────
	// SelfHitTestInvisible → 캔버스 자체는 입력 미수신, 자식(버튼)만 수신
	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(
		UCanvasPanel::StaticClass(), TEXT("NotifRoot"));
	WidgetTree->RootWidget = Canvas;
	Canvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	// ── 배너 버튼 (어두운 반투명 배경) ──────────────────────────────────
	BannerButton = WidgetTree->ConstructWidget<UButton>(
		UButton::StaticClass(), TEXT("BannerBtn"));

	FButtonStyle Style = BannerButton->GetStyle();
	{
		FSlateBrush B;
		B.TintColor = FSlateColor(FLinearColor(0.04f, 0.04f, 0.04f, 0.92f));
		Style.Normal = B;
	}
	{
		FSlateBrush B;
		B.TintColor = FSlateColor(FLinearColor(0.13f, 0.13f, 0.13f, 0.95f));
		Style.Hovered = B;
	}
	{
		FSlateBrush B;
		B.TintColor = FSlateColor(FLinearColor(0.22f, 0.22f, 0.22f, 0.97f));
		Style.Pressed = B;
	}
	Style.NormalPadding  = FMargin(14.f, 10.f);
	Style.PressedPadding = FMargin(14.f, 10.f);
	BannerButton->SetStyle(Style);
	BannerButton->SetVisibility(ESlateVisibility::Collapsed); // 기본 숨김
	BannerButton->OnClicked.AddDynamic(this, &UDeliveryNotifBannerWidget::OnButtonClicked);

	// ── 버튼 내부: HBox (아이콘 | 텍스트 VBox) ──────────────────────────
	UHorizontalBox* HBox = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("NotifHBox"));
	BannerButton->SetContent(HBox);

	// 아이콘 이미지
	IconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("NotifIcon"));
	IconImage->SetDesiredSizeOverride(FVector2D(40.f, 40.f));
	{
		UHorizontalBoxSlot* S = Cast<UHorizontalBoxSlot>(HBox->AddChild(IconImage));
		if (S)
		{
			S->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));
			S->SetVerticalAlignment(VAlign_Center);
		}
	}

	// 텍스트 VBox
	UVerticalBox* TextVBox = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("NotifTextVBox"));
	{
		UHorizontalBoxSlot* S = Cast<UHorizontalBoxSlot>(HBox->AddChild(TextVBox));
		if (S)
		{
			FSlateChildSize Fill;
			Fill.SizeRule = ESlateSizeRule::Fill;
			Fill.Value    = 1.f;
			S->SetSize(Fill);
			S->SetVerticalAlignment(VAlign_Center);
		}
	}

	// 타이틀 "🛵 새 배달 요청" (주황)
	UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("NotifTitle"));
	TitleText->SetText(FText::FromString(TEXT("새 배달 요청")));
	TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.42f, 0.21f, 1.f)));
	{
		FSlateFontInfo F = TitleText->GetFont();
		F.Size = 14;
		F.TypefaceFontName = FName("Bold");
		TitleText->SetFont(F);
	}
	{
		UVerticalBoxSlot* S = Cast<UVerticalBoxSlot>(TextVBox->AddChild(TitleText));
		if (S) S->SetPadding(FMargin(0.f, 0.f, 0.f, 3.f));
	}

	// 설명 텍스트 (ShowBanner에서 갱신)
	DescText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("NotifDesc"));
	DescText->SetText(FText::GetEmpty());
	DescText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f, 1.f)));
	{
		FSlateFontInfo F = DescText->GetFont();
		F.Size = 12;
		DescText->SetFont(F);
	}
	{
		UVerticalBoxSlot* S = Cast<UVerticalBoxSlot>(TextVBox->AddChild(DescText));
		if (S) S->SetPadding(FMargin(0.f, 0.f, 0.f, 2.f));
	}

	// 힌트 "탭하여 확인" (연한 회색, 소자)
	UTextBlock* HintText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("NotifHint"));
	HintText->SetText(FText::FromString(TEXT("탭하여 확인 →")));
	HintText->SetColorAndOpacity(FSlateColor(FLinearColor(0.55f, 0.55f, 0.55f, 1.f)));
	{
		FSlateFontInfo F = HintText->GetFont();
		F.Size = 10;
		HintText->SetFont(F);
	}
	TextVBox->AddChild(HintText);

	// ── 캔버스에 배너 버튼 배치 (우하단 폰 위젯 위에 고정) ─────────────
	// 폰 위젯이 우하단에 있으므로 배너도 같은 위치 근처에 표시해
	// "알림이 폰에 들어오는" 느낌을 준다.
	UCanvasPanelSlot* BtnSlot = Cast<UCanvasPanelSlot>(Canvas->AddChild(BannerButton));
	if (BtnSlot)
	{
		BtnSlot->SetAnchors(FAnchors(1.f, 1.f, 1.f, 1.f)); // 우하단 앵커
		BtnSlot->SetAlignment(FVector2D(1.f, 1.f));         // 우하단 기준 정렬
		// 폰 위젯 높이만큼 위로 올림 (폰이 약 420px 높이라 가정, -440 = 폰 위)
		BtnSlot->SetPosition(FVector2D(-10.f, -440.f));
		BtnSlot->SetSize(FVector2D(280.f, 80.f));           // 폰 너비에 맞춘 크기
		BtnSlot->SetZOrder(0);
	}
}

// ────────────────────────────────────────────────────────────────────────────────
// ShowBanner
// ────────────────────────────────────────────────────────────────────────────────
void UDeliveryNotifBannerWidget::ShowBanner(const FText& Description,
                                             UTexture2D* Icon,
                                             FLinearColor AccentColor)
{
	if (!BannerButton) return;

	// 설명 텍스트 갱신
	if (DescText)
		DescText->SetText(Description);

	// 아이콘 갱신
	if (IconImage)
	{
		if (Icon)
		{
			IconImage->SetBrushFromTexture(Icon);
			IconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			IconImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 배너 표시
	BannerButton->SetVisibility(ESlateVisibility::Visible);

	// 기존 타이머 취소 후 5초 후 자동 숨김
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(AutoHideTimer);
		World->GetTimerManager().SetTimer(
			AutoHideTimer,
			this, &UDeliveryNotifBannerWidget::HideBanner,
			10.f,
			false
		);
	}
}

// ────────────────────────────────────────────────────────────────────────────────
// HideBanner
// ────────────────────────────────────────────────────────────────────────────────
void UDeliveryNotifBannerWidget::HideBanner()
{
	if (BannerButton)
		BannerButton->SetVisibility(ESlateVisibility::Collapsed);
}

// ────────────────────────────────────────────────────────────────────────────────
// OnButtonClicked
// ────────────────────────────────────────────────────────────────────────────────
void UDeliveryNotifBannerWidget::OnButtonClicked()
{
	// 타이머 취소 + 배너 즉시 숨기기
	UWorld* World = GetWorld();
	if (World)
		World->GetTimerManager().ClearTimer(AutoHideTimer);

	HideBanner();

	// 상위(DeliveryPhoneWidget)에 클릭 알림 → OpenDeliveryApp() 호출됨
	OnBannerClicked.Broadcast();
}
