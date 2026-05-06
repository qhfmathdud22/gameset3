#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SkyTimeController.generated.h"

// 레벨에 하나 배치 → SunLight 에 DirectionalLight 연결
// 게임 시간에 맞춰 태양을 회전시켜 아침/낮/저녁/밤 연출
UCLASS()
class GAMESET_API ASkyTimeController : public AActor
{
	GENERATED_BODY()

public:
	ASkyTimeController();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

public:
	// 레벨에 있는 Directional Light를 여기에 할당
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Sky")
	class ADirectionalLight* SunLight;

	// 선택: Sky Atmosphere / Sky Light 등 추가 회전 대상
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Sky")
	AActor* SkySphere;

	// 태양 회전 속도 부스터 (1.0 = 정상, >1 = 빨라짐)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
	float SunAngleOffset = 0.f;   // 시작 yaw 보정

	// 현재 게임 시각 (Blueprint에서 참조 가능)
	UPROPERTY(BlueprintReadOnly, Category = "Sky")
	float CurrentGameHour = 6.f;

private:
	UPROPERTY() class UDeliveryComponent* CachedDC = nullptr;

	class UDeliveryComponent* FindPlayerDC();
};
