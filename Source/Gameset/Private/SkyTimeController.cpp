#include "SkyTimeController.h"
#include "DeliveryComponent.h"
#include "Engine/DirectionalLight.h"
#include "Kismet/GameplayStatics.h"

ASkyTimeController::ASkyTimeController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASkyTimeController::BeginPlay()
{
	Super::BeginPlay();
	CachedDC = FindPlayerDC();
}

UDeliveryComponent* ASkyTimeController::FindPlayerDC()
{
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	return Pawn ? Pawn->FindComponentByClass<UDeliveryComponent>() : nullptr;
}

void ASkyTimeController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CachedDC)
	{
		CachedDC = FindPlayerDC();
		if (!CachedDC) return;
	}

	CurrentGameHour = CachedDC->GetGameHour();

	if (!SunLight) return;

	// Pitch: 6시 → 0°, 12시 → -90°, 18시 → 0° (수평), 0시 → +90° (지평선 아래)
	const float Phase = (CurrentGameHour - 6.f) / 24.f * 2.f * PI;
	const float Pitch = -FMath::Sin(Phase) * 90.f;
	const float Yaw   = (CurrentGameHour / 24.f) * 360.f + SunAngleOffset;

	SunLight->SetActorRotation(FRotator(Pitch, Yaw, 0.f));

	if (SkySphere)
	{
		// Sky sphere 는 yaw 만 회전
		SkySphere->SetActorRotation(FRotator(0.f, Yaw, 0.f));
	}
}
