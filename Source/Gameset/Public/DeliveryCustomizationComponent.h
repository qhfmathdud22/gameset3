#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DeliveryTypes.h"
#include "DeliveryCustomizationComponent.generated.h"

// 업그레이드 레벨별 Mutable 파라미터 매핑
// 에디터에서 CO_DeliveryCharacter를 만든 후 여기 할당
UCLASS(ClassGroup=(Delivery), meta=(BlueprintSpawnableComponent))
class GAMESET_API UDeliveryCustomizationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDeliveryCustomizationComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ── Mutable 설정 ─────────────────────────────────────────────────
	// 에디터에서 CO_DeliveryCharacter 할당
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mutable")
	class UCustomizableObject* DeliveryCharacterCO;

	UPROPERTY(BlueprintReadOnly, Category = "Mutable")
	class UCustomizableObjectInstance* CharacterInstance;

	// ── 업그레이드 → 외형 변경 ───────────────────────────────────────

	// 업그레이드 구매 시 호출 → 외형 자동 변경
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void OnUpgradePurchased(EUpgradeType Type, int32 NewLevel);

	// 게임 시작 시 기본 외형 적용
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void ApplyDefaultAppearance();

	// 외형 업데이트 요청 (비동기)
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void UpdateAppearance();

	// ── Mutable 파라미터 이름 설정 ───────────────────────────────────
	// CO_DeliveryCharacter 에서 만든 파라미터 이름과 일치시켜야 함

	UPROPERTY(EditDefaultsOnly, Category = "Mutable|Params")
	FString ParamHelmet = TEXT("Helmet");      // None / Basic / Premium

	UPROPERTY(EditDefaultsOnly, Category = "Mutable|Params")
	FString ParamUniform = TEXT("Uniform");    // Basic / Company / Premium

	UPROPERTY(EditDefaultsOnly, Category = "Mutable|Params")
	FString ParamBag = TEXT("Bag");            // Small / Medium / Large

	UPROPERTY(EditDefaultsOnly, Category = "Mutable|Params")
	FString ParamSunglasses = TEXT("Sunglasses"); // None / Basic / AR

	UPROPERTY(EditDefaultsOnly, Category = "Mutable|Params")
	FString ParamSkinColor = TEXT("SkinColor");

private:
	// 현재 업그레이드 레벨 캐시
	int32 BagLevel        = 0;
	int32 SunglassesLevel = 0;

	UFUNCTION()
	void OnInstanceUpdated(class UCustomizableObjectInstance* Instance);
};
