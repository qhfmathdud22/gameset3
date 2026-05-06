#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DeliveryHUD.generated.h"

UCLASS()
class GAMESET_API ADeliveryHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawMoneyDisplay   (class UDeliveryComponent* DC);
	void DrawVitalBars      (class UDeliveryComponent* DC);   // 왼쪽 하단 3스탯
	void DrawOrderList      (class UDeliveryComponent* DC);
	void DrawInteractPrompt (class UDeliveryComponent* DC);
	void DrawStats          ();
	void DrawMinimap        (class UDeliveryComponent* DC);   // 선글라스 Lv1+
	void DrawSingleBar      (float X, float Y, float W, float H, float Ratio,
	                         const FLinearColor& FillColor, const FString& Label);
};
