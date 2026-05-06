#include "UpgradeComponent.h"
#include "DeliveryComponent.h"
#include "DeliveryCustomizationComponent.h"
#include "BagInventoryComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UUpgradeComponent::UUpgradeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	InitDefaults();
}

void UUpgradeComponent::InitDefaults()
{
	auto Add = [&](EUpgradeType T, const TCHAR* Name, const TCHAR* Desc, float Cost, float Scale)
	{
		FUpgradeLevel Up;
		Up.DisplayName = FText::FromString(Name);
		Up.Description = FText::FromString(Desc);
		Up.BaseCost    = Cost;
		Up.CostScale   = Scale;
		Upgrades.Add(T, Up);
	};

	Add(EUpgradeType::SpeedBoost,    TEXT("Speed Boost"),    TEXT("이동속도 +15% (최대 3단계)"),               300.f, 1.8f);
	Add(EUpgradeType::StaminaMax,    TEXT("Stamina Pack"),   TEXT("최대 스태미나 +30 (최대 3단계)"),            400.f, 1.6f);
	Add(EUpgradeType::MultiOrder,    TEXT("Extra Bag"),      TEXT("동시 배달 +1개 (최대 3단계)"),               600.f, 2.0f);
	Add(EUpgradeType::TimeBonus,     TEXT("Time Bonus"),     TEXT("배달 제한시간 +20초 (최대 3단계)"),          250.f, 1.5f);
	Add(EUpgradeType::TipMultiplier, TEXT("Tip Master"),     TEXT("배달 보상금 +20% (최대 3단계)"),             500.f, 1.7f);
	Add(EUpgradeType::Sunglasses,    TEXT("AR Sunglasses"),  TEXT("미니맵+네비 해금 (3단계 → 마커↑/화살표)"),  800.f, 2.2f);
	Add(EUpgradeType::Shoes,         TEXT("Running Shoes"),  TEXT("달리기 속도 +10% (최대 3단계)"),             400.f, 1.9f);
}

void UUpgradeComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UUpgradeComponent::TryPurchaseUpgrade(EUpgradeType Type)
{
	if (!Upgrades.Contains(Type)) return false;

	FUpgradeLevel& Up = Upgrades[Type];
	if (!Up.CanUpgrade()) return false;

	// 돈은 같은 오너의 DeliveryComponent에서 차감
	UDeliveryComponent* DC = GetOwner() ? GetOwner()->FindComponentByClass<UDeliveryComponent>() : nullptr;
	if (!DC || !DC->SpendMoney(Up.GetNextCost())) return false;

	Up.Level++;
	ApplyUpgrade(Type);
	OnUpgradePurchased.Broadcast(Type);
	return true;
}

void UUpgradeComponent::ApplyUpgrade(EUpgradeType Type)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	UDeliveryComponent* DC = OwnerActor->FindComponentByClass<UDeliveryComponent>();

	// Mutable 외형 컴포넌트 (있을 때만 연동)
	UDeliveryCustomizationComponent* Cust =
		OwnerActor->FindComponentByClass<UDeliveryCustomizationComponent>();

	switch (Type)
	{
	case EUpgradeType::SpeedBoost:
		RecalculateWalkSpeed();
		// 스프린트 임계값 갱신
		if (DC) DC->UpdateSprintThreshold();
		// 유니폼 변경
		if (Cust) Cust->OnUpgradePurchased(EUpgradeType::SpeedBoost,
		                                    GetUpgradeLevel(EUpgradeType::SpeedBoost));
		break;

	case EUpgradeType::StaminaMax:
		if (DC) DC->ApplyStaminaUpgrade(GetStaminaMultiplier());
		break;

	case EUpgradeType::MultiOrder:
		if (DC) DC->MaxCarryOrders = GetMaxOrders();
		// 가방 격자 확장 (4×4 → 5×5 → 6×6 → 7×8), 무게도 증가
		if (UBagInventoryComponent* Bag = OwnerActor->FindComponentByClass<UBagInventoryComponent>())
		{
			const int32 Lv = GetUpgradeLevel(EUpgradeType::MultiOrder);
			const int32 Dim[]    = { 4, 5, 6, 7 };
			const int32 DimH[]   = { 4, 5, 6, 8 };
			const float MaxWt[]  = { 30.f, 45.f, 65.f, 90.f };
			const int32 Idx = FMath::Clamp(Lv, 0, 3);
			Bag->ResizeGrid(Dim[Idx], DimH[Idx], MaxWt[Idx]);
		}
		// 외형(가방 메시) 변경
		if (Cust) Cust->OnUpgradePurchased(EUpgradeType::MultiOrder,
		                                    GetUpgradeLevel(EUpgradeType::MultiOrder));
		break;

	case EUpgradeType::Sunglasses:
		if (DC) DC->SunglassesLevel = GetUpgradeLevel(EUpgradeType::Sunglasses);
		if (Cust) Cust->OnUpgradePurchased(EUpgradeType::Sunglasses,
		                                    GetUpgradeLevel(EUpgradeType::Sunglasses));
		break;

	case EUpgradeType::Shoes:
		if (DC) DC->ShoesLevel = GetUpgradeLevel(EUpgradeType::Shoes);
		RecalculateWalkSpeed();
		// 스프린트 임계값 갱신
		if (DC) DC->UpdateSprintThreshold();
		break;

	default: break;
	}
}

int32 UUpgradeComponent::GetUpgradeLevel(EUpgradeType Type) const
{
	const FUpgradeLevel* Up = Upgrades.Find(Type);
	return Up ? Up->Level : 0;
}

float UUpgradeComponent::GetUpgradeCost(EUpgradeType Type) const
{
	const FUpgradeLevel* Up = Upgrades.Find(Type);
	return Up ? Up->GetNextCost() : 0.f;
}

bool UUpgradeComponent::CanPurchaseUpgrade(EUpgradeType Type) const
{
	const FUpgradeLevel* Up = Upgrades.Find(Type);
	return Up && Up->CanUpgrade();
}

void UUpgradeComponent::RecalculateWalkSpeed()
{
	// SpeedBoost와 Shoes 업그레이드의 곱하기 효과를 모두 반영하여 MaxWalkSpeed 설정
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	if (ACharacter* Char = Cast<ACharacter>(OwnerActor))
	{
		if (UCharacterMovementComponent* CMC = Char->GetCharacterMovement())
		{
			UDeliveryComponent* DC = OwnerActor->FindComponentByClass<UDeliveryComponent>();
			float SpeedMultiplier = GetSpeedMultiplier();
			float ShoesMultiplier = DC ? DC->GetShoesSpeedMultiplier() : 1.f;
			CMC->MaxWalkSpeed = BaseWalkSpeed * SpeedMultiplier * ShoesMultiplier;
		}
	}
}

float UUpgradeComponent::GetSpeedMultiplier()   const { return 1.f + GetUpgradeLevel(EUpgradeType::SpeedBoost)    * 0.15f; }
float UUpgradeComponent::GetStaminaMultiplier() const { return 1.f + GetUpgradeLevel(EUpgradeType::StaminaMax)    * 0.30f; }
int32 UUpgradeComponent::GetMaxOrders()         const { return 1   + GetUpgradeLevel(EUpgradeType::MultiOrder);            }
float UUpgradeComponent::GetTimeBonusSec()      const { return       GetUpgradeLevel(EUpgradeType::TimeBonus)     * 20.f;  }
float UUpgradeComponent::GetRewardMultiplier()  const { return 1.f + GetUpgradeLevel(EUpgradeType::TipMultiplier) * 0.20f; }
