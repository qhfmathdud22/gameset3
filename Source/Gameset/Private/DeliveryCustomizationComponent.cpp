#include "DeliveryCustomizationComponent.h"
#include "MuCO/CustomizableObject.h"
#include "MuCO/CustomizableObjectInstance.h"

UDeliveryCustomizationComponent::UDeliveryCustomizationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDeliveryCustomizationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!DeliveryCharacterCO) return;

	// CO에서 인스턴스 생성
	CharacterInstance = DeliveryCharacterCO->CreateInstance();
	if (!CharacterInstance) return;

	// 업데이트 완료 콜백 등록
	CharacterInstance->UpdatedDelegate.AddDynamic(this, &UDeliveryCustomizationComponent::OnInstanceUpdated);

	// 기본 외형 적용
	ApplyDefaultAppearance();
}

void UDeliveryCustomizationComponent::ApplyDefaultAppearance()
{
	if (!CharacterInstance) return;

	// 기본값: 가장 낮은 옵션
	CharacterInstance->SetIntParameterSelectedOption(ParamHelmet,     TEXT("None"));
	CharacterInstance->SetIntParameterSelectedOption(ParamUniform,    TEXT("Basic"));
	CharacterInstance->SetIntParameterSelectedOption(ParamBag,        TEXT("Small"));
	CharacterInstance->SetIntParameterSelectedOption(ParamSunglasses, TEXT("None"));

	UpdateAppearance();
}

void UDeliveryCustomizationComponent::OnUpgradePurchased(EUpgradeType Type, int32 NewLevel)
{
	if (!CharacterInstance) return;

	switch (Type)
	{
	// 가방 크기 → MultiOrder 업그레이드와 연동
	case EUpgradeType::MultiOrder:
		BagLevel = NewLevel;
		{
			const TCHAR* BagOptions[] = { TEXT("Small"), TEXT("Medium"), TEXT("Large") };
			int32 Idx = FMath::Clamp(NewLevel - 1, 0, 2);
			CharacterInstance->SetIntParameterSelectedOption(ParamBag, BagOptions[Idx]);
		}
		break;

	// 선글라스 → AR 선글라스 업그레이드
	case EUpgradeType::Sunglasses:
		SunglassesLevel = NewLevel;
		{
			const TCHAR* GlassOptions[] = { TEXT("Basic"), TEXT("Tinted"), TEXT("AR") };
			int32 Idx = FMath::Clamp(NewLevel - 1, 0, 2);
			CharacterInstance->SetIntParameterSelectedOption(ParamSunglasses, GlassOptions[Idx]);
		}
		break;

	// 스피드 업그레이드 → 프리미엄 유니폼
	case EUpgradeType::SpeedBoost:
		if (NewLevel >= 2)
			CharacterInstance->SetIntParameterSelectedOption(ParamUniform, TEXT("Company"));
		if (NewLevel >= 3)
			CharacterInstance->SetIntParameterSelectedOption(ParamUniform, TEXT("Premium"));
		break;

	default: break;
	}

	UpdateAppearance();
}

void UDeliveryCustomizationComponent::UpdateAppearance()
{
	if (!CharacterInstance) return;
	// Mutable 비동기 메시 재생성 요청
	CharacterInstance->UpdateSkeletalMeshAsync();
}

void UDeliveryCustomizationComponent::OnInstanceUpdated(UCustomizableObjectInstance* Instance)
{
	// 메시 업데이트 완료 — 필요시 Blueprint 이벤트 트리거 가능
	UE_LOG(LogTemp, Log, TEXT("[Delivery] 캐릭터 외형 업데이트 완료"));
}
