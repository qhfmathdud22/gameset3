#include "DeliveryHUD.h"
#include "DeliveryComponent.h"
#include "BagInventoryComponent.h"
#include "PackageActor.h"
#include "DeliveryPointActor.h"
#include "DeliveryManagerComponent.h"
#include "GamesetGameMode.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

namespace
{
	static const FLinearColor ColorGold  = FLinearColor(1.f, 0.85f, 0.f);
	static const FLinearColor ColorWhite = FLinearColor::White;
	static const FLinearColor ColorRed   = FLinearColor::Red;
	static const FLinearColor ColorGreen = FLinearColor(0.1f, 0.9f, 0.2f);
	static const FLinearColor ColorGray  = FLinearColor(0.15f, 0.15f, 0.15f, 0.7f);
	static const FLinearColor ColorDark  = FLinearColor(0.f, 0.f, 0.f, 0.55f);
}

void ADeliveryHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas) return;

	// 어떤 캐릭터든 UDeliveryComponent 찾기
	APawn* Pawn = GetOwningPawn();
	UDeliveryComponent* DC = Pawn ? Pawn->FindComponentByClass<UDeliveryComponent>() : nullptr;
	if (!DC) return;

	DrawMoneyDisplay(DC);
	DrawVitalBars(DC);
	DrawOrderList(DC);
	DrawInteractPrompt(DC);
	DrawStats();
	DrawMinimap(DC);
}

void ADeliveryHUD::DrawMoneyDisplay(UDeliveryComponent* DC)
{
	const float W = Canvas->SizeX;
	FString MoneyStr = FString::Printf(TEXT("₩ %.0f"), DC->Money);

	DrawRect(ColorDark, W - 220.f, 12.f, 205.f, 42.f);
	DrawText(MoneyStr, ColorGold, W - 210.f, 20.f, nullptr, 1.6f);
}

void ADeliveryHUD::DrawSingleBar(float X, float Y, float W, float H, float Ratio,
                                  const FLinearColor& FillColor, const FString& Label)
{
	Ratio = FMath::Clamp(Ratio, 0.f, 1.f);
	DrawRect(ColorGray, X - 2.f, Y - 2.f, W + 4.f, H + 4.f);
	DrawRect(FillColor, X, Y, W * Ratio, H);
	DrawText(Label, ColorWhite, X, Y - 16.f, nullptr, 0.85f);
}

void ADeliveryHUD::DrawVitalBars(UDeliveryComponent* DC)
{
	// 왼쪽 하단 3스탯 + 무게
	const float H  = Canvas->SizeY;
	const float BW = 180.f;
	const float BH = 12.f;
	const float X  = 20.f;
	float Y        = H - 150.f;
	const float RowH = 32.f;

	// 스태미나
	{
		float Ratio = DC->MaxStamina > 0.f ? DC->CurrentStamina / DC->MaxStamina : 0.f;
		FString Label;
		FLinearColor Col = FLinearColor(0.2f, 0.9f, 0.3f);
		if (DC->bSprintLocked)
		{
			Label = FString::Printf(TEXT("스태미나 [잠금 %.0f초]"), DC->SprintLockTimer);
			Col   = ColorRed;
		}
		else
		{
			Label = FString::Printf(TEXT("스태미나  %.0f / %.0f"), DC->CurrentStamina, DC->MaxStamina);
		}
		DrawSingleBar(X, Y, BW, BH, Ratio, Col, Label);
		Y += RowH;
	}

	// 배고픔
	{
		float Ratio = DC->Hunger / DC->HungerMax;
		FLinearColor Col = FLinearColor(0.95f, 0.55f, 0.15f);   // 주황
		FString Label    = FString::Printf(TEXT("배고픔  %.0f / %.0f"), DC->Hunger, DC->HungerMax);
		DrawSingleBar(X, Y, BW, BH, Ratio, Col, Label);
		Y += RowH;
	}

	// 갈증
	{
		float Ratio = DC->Thirst / DC->ThirstMax;
		FLinearColor Col = FLinearColor(0.2f, 0.6f, 1.f);        // 파랑
		FString Label    = FString::Printf(TEXT("갈증  %.0f / %.0f"), DC->Thirst, DC->ThirstMax);
		DrawSingleBar(X, Y, BW, BH, Ratio, Col, Label);
		Y += RowH;
	}

	// 가방 무게
	if (AActor* OwnerActor = DC->GetOwner())
	{
		if (UBagInventoryComponent* Bag = OwnerActor->FindComponentByClass<UBagInventoryComponent>())
		{
			float Ratio = Bag->GetWeightRatio();
			FLinearColor Col = FLinearColor::LerpUsingHSV(
				FLinearColor(0.7f, 0.7f, 0.7f), ColorRed, Ratio);
			FString Label = FString::Printf(TEXT("가방  %.1f / %.1f kg"),
				Bag->GetTotalWeight(), Bag->MaxWeight);
			DrawSingleBar(X, Y, BW, BH, Ratio, Col, Label);
		}
	}
}

void ADeliveryHUD::DrawOrderList(UDeliveryComponent* DC)
{
	if (DC->CarriedPackages.IsEmpty()) return;

	float X = 20.f;
	float Y = 20.f;

	DrawRect(ColorDark, X - 5.f, Y - 5.f, 350.f, 30.f + DC->CarriedPackages.Num() * 60.f);
	DrawText(TEXT("[ 배달 중인 오더 ]"), ColorGold, X, Y, nullptr, 1.1f);
	Y += 28.f;

	for (APackageActor* Package : DC->CarriedPackages)
	{
		if (!Package) continue;
		const FDeliveryOrder& Order = Package->Order;

		FString Line1 = FString::Printf(TEXT("%s  →  ₩%.0f"),
			*Order.CustomerName.ToString(), Order.Reward);
		DrawText(Line1, ColorWhite, X, Y, nullptr, 1.0f);
		Y += 20.f;

		FLinearColor TimeColor = Order.TimeRemaining < 20.f ? ColorRed : ColorWhite;
		FString Line2 = FString::Printf(TEXT("남은 시간: %.0f초  [%s]"),
			Order.TimeRemaining, *Order.OrderID);
		DrawText(Line2, TimeColor, X, Y, nullptr, 0.95f);
		Y += 40.f;
	}
}

void ADeliveryHUD::DrawInteractPrompt(UDeliveryComponent* DC)
{
	FString Prompt;
	if (DC->HasNearbyDropoff() && DC->CarriedPackages.Num() > 0)
		Prompt = TEXT("[ E ]  배달 완료");
	else if (DC->HasNearbyPackage() && DC->CarriedPackages.Num() < DC->MaxCarryOrders)
		Prompt = TEXT("[ E ]  픽업하기");
	else
		return;

	const float W  = Canvas->SizeX;
	const float H  = Canvas->SizeY;
	const float BW = 280.f;
	const float BH = 44.f;
	const float BX = (W - BW) * 0.5f;
	const float BY = H * 0.72f;

	DrawRect(ColorDark,  BX - 8.f, BY - 8.f, BW + 16.f, BH + 16.f);
	DrawText(Prompt, ColorGold, BX, BY + 10.f, nullptr, 1.4f);
}

void ADeliveryHUD::DrawMinimap(UDeliveryComponent* DC)
{
	// 선글라스 Lv1 이상일 때만 표시
	if (DC->SunglassesLevel < 1) return;

	const float H     = Canvas->SizeY;
	const float MapR  = 90.f;          // 미니맵 반지름 (픽셀)
	const float CX    = 110.f;         // 미니맵 중심 X (왼쪽 하단)
	const float CY    = H - 120.f;     // 미니맵 중심 Y
	const float Scale = 3000.f;        // 반지름에 대응하는 월드 거리 (유닛)

	// ── 배경 ─────────────────────────────────────────────────────────
	// 테두리 (하늘색 AR느낌)
	const FLinearColor ColBorder(0.3f, 0.85f, 1.f, 0.6f);
	const FLinearColor ColBg    (0.f,  0.f,   0.f, 0.70f);

	DrawRect(ColBg,     CX - MapR,       CY - MapR,       MapR * 2.f,       MapR * 2.f);
	DrawRect(ColBorder, CX - MapR - 2.f, CY - MapR - 2.f, MapR * 2.f + 4.f, 2.f);
	DrawRect(ColBorder, CX - MapR - 2.f, CY + MapR,       MapR * 2.f + 4.f, 2.f);
	DrawRect(ColBorder, CX - MapR - 2.f, CY - MapR - 2.f, 2.f, MapR * 2.f + 4.f);
	DrawRect(ColBorder, CX + MapR,       CY - MapR - 2.f, 2.f, MapR * 2.f + 4.f);

	// ── 레이블 ───────────────────────────────────────────────────────
	DrawText(TEXT("MAP"), ColBorder, CX - 14.f, CY - MapR - 16.f, nullptr, 0.85f);

	APawn* Pawn = GetOwningPawn();
	if (!Pawn) return;

	const FVector PlayerPos = Pawn->GetActorLocation();

	// 월드 좌표 → 미니맵 픽셀 (UE 기본: X = 북, Y = 동)
	// 미니맵: 동 = 오른쪽, 북 = 위
	auto WorldToMap = [&](FVector WorldPos) -> FVector2D
	{
		FVector Delta = WorldPos - PlayerPos;
		FVector2D Raw(CX + (Delta.Y / Scale) * MapR,
		              CY - (Delta.X / Scale) * MapR);
		// 미니맵 경계 밖이면 가장자리로 클램프
		FVector2D Offset = Raw - FVector2D(CX, CY);
		float Dist = Offset.Size();
		if (Dist > MapR - 5.f)
			Raw = FVector2D(CX, CY) + Offset.GetSafeNormal() * (MapR - 5.f);
		return Raw;
	};

	// ── Lv1 : 픽업 포인트 (노란 점) ──────────────────────────────────
	UWorld* World = GetWorld();
	if (!World) return;

	if (AGamesetGameMode* GM = Cast<AGamesetGameMode>(World->GetAuthGameMode()))
	{
		if (GM->DeliveryManager)
		{
			for (APackageActor* Pkg : GM->DeliveryManager->PickupPoints)
			{
				if (!Pkg) continue;
				FVector2D MP = WorldToMap(Pkg->GetActorLocation());
				DrawRect(FLinearColor(1.f, 0.9f, 0.f), MP.X - 4.f, MP.Y - 4.f, 8.f, 8.f);
			}

			// ── Lv2 : 드롭오프 포인트 (초록 점) ───────────────────────
			if (DC->SunglassesLevel >= 2)
			{
				for (ADeliveryPointActor* Drop : GM->DeliveryManager->DropoffPoints)
				{
					if (!Drop || !Drop->bIsActive) continue;
					FVector2D MP = WorldToMap(Drop->GetActorLocation());
					DrawRect(FLinearColor(0.2f, 1.f, 0.35f), MP.X - 4.f, MP.Y - 4.f, 8.f, 8.f);
				}
			}
		}
	}

	// ── 플레이어 중심 점 (흰색) ───────────────────────────────────────
	DrawRect(FLinearColor::White, CX - 5.f, CY - 5.f, 10.f, 10.f);

	// ── Lv3 : 네비게이션 화살표 ──────────────────────────────────────
	if (DC->SunglassesLevel >= 3)
	{
		AActor* Target = nullptr;
		float   MinDist = MAX_FLT;

		if (DC->CarriedPackages.Num() > 0)
		{
			// 배달 중 → 드롭오프 방향
			for (APackageActor* Pkg : DC->CarriedPackages)
			{
				if (!Pkg || !Pkg->Order.DropoffActor) continue;
				float D = FVector::Dist(PlayerPos, Pkg->Order.DropoffActor->GetActorLocation());
				if (D < MinDist) { MinDist = D; Target = Pkg->Order.DropoffActor; }
			}
		}

		if (Target)
		{
			// 월드 방향 → 미니맵 화면 방향
			FVector WorldDir = (Target->GetActorLocation() - PlayerPos).GetSafeNormal2D();
			// 화면: 동(Y)→오른쪽, 북(X)→위
			FVector2D ScreenDir( WorldDir.Y, -WorldDir.X);
			FVector2D ScreenPerp( WorldDir.X,  WorldDir.Y);

			const float ArrLen = 22.f;
			const FLinearColor ColArrow(0.2f, 1.f, 0.4f);

			FVector2D ArrowTip(CX + ScreenDir.X * ArrLen, CY + ScreenDir.Y * ArrLen);
			FVector2D HeadL   = ArrowTip - ScreenDir * 8.f + ScreenPerp * 5.f;
			FVector2D HeadR   = ArrowTip - ScreenDir * 8.f - ScreenPerp * 5.f;

			DrawLine(CX, CY, ArrowTip.X, ArrowTip.Y, ColArrow, 2.5f);
			DrawLine(ArrowTip.X, ArrowTip.Y, HeadL.X, HeadL.Y, ColArrow, 2.5f);
			DrawLine(ArrowTip.X, ArrowTip.Y, HeadR.X, HeadR.Y, ColArrow, 2.5f);

			// 목표까지 거리 표시
			FString DistStr = FString::Printf(TEXT("%.0fm"), MinDist / 100.f);
			DrawText(DistStr, ColArrow, CX - 15.f, CY + MapR + 4.f, nullptr, 0.85f);
		}
	}
}

void ADeliveryHUD::DrawStats()
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;

	// GetWorld() nullptr 체크 추가
	UWorld* World = GetWorld();
	if (!World) return;

	if (AGamesetGameMode* GM = Cast<AGamesetGameMode>(World->GetAuthGameMode()))
	{
		if (GM->DeliveryManager)
		{
			int32 Active = GM->DeliveryManager->GetActiveOrders().Num();
			int32 Done   = GM->DeliveryManager->TotalDelivered;
			int32 Failed = GM->DeliveryManager->TotalFailed;

			FString Info = FString::Printf(TEXT("대기: %d  완료: %d  실패: %d"), Active, Done, Failed);
			DrawRect(ColorDark, W - 340.f, H - 30.f, 325.f, 24.f);
			DrawText(Info, ColorWhite, W - 333.f, H - 27.f, nullptr, 0.9f);
		}
	}
}
