#include "DeliveryComponent.h"
#include "DeliveryPhoneWidget.h"
#include "BagInventoryComponent.h"
#include "PackageActor.h"
#include "DeliveryPointActor.h"
#include "GamesetGameMode.h"
#include "DeliveryManagerComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

UDeliveryComponent::UDeliveryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDeliveryComponent::BeginPlay()
{
	Super::BeginPlay();
	MaxStamina     = BaseMaxStamina;
	CurrentStamina = MaxStamina;
	Hunger         = HungerMax;
	Thirst         = ThirstMax;

	// 시작: 2026/04/19 오전 6:00
	GameTimeSeconds = GameDayDuration * (6.f / 24.f);
}

void UDeliveryComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bInputBound)
		TryBindInput();

	UpdateStats(DeltaTime);
	UpdateGameClock(DeltaTime);
}

// ── 게임 시계 ─────────────────────────────────────────────────────────

void UDeliveryComponent::UpdateGameClock(float DeltaTime)
{
	GameTimeSeconds += DeltaTime;
	if (GameTimeSeconds >= GameDayDuration)
	{
		GameTimeSeconds -= GameDayDuration;
		GameDay++;
	}
}

// ── Input 바인딩 ──────────────────────────────────────────────────────

void UDeliveryComponent::TryBindInput()
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (!Char) return;

	APlayerController* PC = Cast<APlayerController>(Char->GetController());
	if (!PC || !PC->InputComponent) return;

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (!EIC) return;

	if (PhoneOpenAction)
		EIC->BindAction(PhoneOpenAction,  ETriggerEvent::Started, this, &UDeliveryComponent::ShowPhone);
	if (PhoneCloseAction)
		EIC->BindAction(PhoneCloseAction, ETriggerEvent::Started, this, &UDeliveryComponent::HidePhone);
	if (InventoryAction)
		EIC->BindAction(InventoryAction,  ETriggerEvent::Started, this, &UDeliveryComponent::ToggleInventory);

	if (PC->IsLocalController())
	{
		if (PhoneWidgetClass)
		{
			PhoneWidgetInstance = CreateWidget<UUserWidget>(PC, PhoneWidgetClass);
			if (PhoneWidgetInstance)
			{
				PhoneWidgetInstance->AddToViewport(10);
				PhoneWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		if (InventoryWidgetClass)
		{
			InventoryWidgetInstance = CreateWidget<UUserWidget>(PC, InventoryWidgetClass);
			if (InventoryWidgetInstance)
			{
				InventoryWidgetInstance->AddToViewport(11);
				InventoryWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}

	bInputBound = true;
}

// ── 스프린트 ──────────────────────────────────────────────────────────

bool UDeliveryComponent::CanSprint() const
{
	return !bSprintLocked && CurrentStamina > 0.f;
}

void UDeliveryComponent::SetSprinting(bool bSprint)
{
	// 스프린트 잠금 상태일 때는 요청을 무시
	if (bSprintLocked) return;

	bIsSprinting = bSprint;

	// 캐릭터 MovementComponent의 MaxWalkSpeed를 스프린트/걷기에 따라 전환
	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement())
		{
			// 달리기 속도: 기본 600 × 총 속도 배율 / 걷기 속도: 300
			const float SprintSpeed = 600.f * GetTotalSpeedMultiplier();
			const float WalkSpeed   = 300.f;
			MoveComp->MaxWalkSpeed  = bSprint ? SprintSpeed : WalkSpeed;
		}
	}
}

// ── 스프린트 임계값 갱신 ──────────────────────────────────────────────

void UDeliveryComponent::UpdateSprintThreshold()
{
	// UpgradeComponent에서 속도 업그레이드 적용 후 이 함수를 호출하여
	// SprintDetectSpeed를 MaxWalkSpeed의 80%로 자동 동기화
	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement())
		{
			SprintDetectSpeed = MoveComp->MaxWalkSpeed * 0.8f;
		}
	}
}

// ── 휴대폰 ────────────────────────────────────────────────────────────

void UDeliveryComponent::ShowPhone()
{
	if (!PhoneWidgetInstance || bPhoneVisible) return;

	bPhoneVisible = true;
	PhoneWidgetInstance->SetVisibility(ESlateVisibility::Visible);

	if (UDeliveryPhoneWidget* PW = Cast<UDeliveryPhoneWidget>(PhoneWidgetInstance))
	{
		PW->bScreenOpen = false;
		PW->ResetToLockScreen();
	}
	SetUIInputMode(true);
}

void UDeliveryComponent::HidePhone()
{
	if (!PhoneWidgetInstance || !bPhoneVisible) return;

	bPhoneVisible = false;
	PhoneWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);

	// 인벤토리도 닫혀있으면 게임 모드로 복귀
	if (!bInventoryVisible) SetUIInputMode(false);
}

// ── 인벤토리 토글 ─────────────────────────────────────────────────────

void UDeliveryComponent::ToggleInventory()
{
	if (!InventoryWidgetInstance) return;

	bInventoryVisible = !bInventoryVisible;
	InventoryWidgetInstance->SetVisibility(
		bInventoryVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (bInventoryVisible)
		SetUIInputMode(true);
	else if (!bPhoneVisible)
		SetUIInputMode(false);
}

void UDeliveryComponent::SetUIInputMode(bool bUIActive)
{
	APlayerController* PC = GetPC();
	if (!PC) return;

	if (bUIActive)
	{
		PC->bShowMouseCursor = true;
		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(Mode);
	}
	else
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}
}

APlayerController* UDeliveryComponent::GetPC() const
{
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
		return Cast<APlayerController>(Pawn->GetController());
	return nullptr;
}

// ── 3스탯 업데이트 ────────────────────────────────────────────────────

void UDeliveryComponent::UpdateStats(float DeltaTime)
{
	// 배고픔·갈증 시간에 따라 감소
	Hunger = FMath::Max(0.f, Hunger - HungerDrainRate * DeltaTime);
	Thirst = FMath::Max(0.f, Thirst - ThirstDrainRate * DeltaTime);

	// 배고픔 또는 갈증이 0이 되면 게임 오버 델리게이트를 한 번만 발동
	if (!bGameOverTriggered && (Hunger <= 0.f || Thirst <= 0.f))
	{
		bGameOverTriggered = true;
		OnGameOver.Broadcast();
	}

	// 활력 비율 (0~1) — 배고픔·갈증 평균
	const float VitalFactor = (Hunger / HungerMax + Thirst / ThirstMax) * 0.5f;

	// 스프린트 잠금 타이머
	if (bSprintLocked)
	{
		SprintLockTimer -= DeltaTime;
		if (SprintLockTimer <= 0.f)
		{
			bSprintLocked   = false;
			SprintLockTimer = 0.f;
		}
	}

	// 실제 이동속도로 달리기 감지 (SprintDetectSpeed 이상 = 달리는 중)
	if (!bSprintLocked)
	{
		if (APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			const float Speed = Pawn->GetVelocity().Size2D();
			bIsSprinting = Speed >= SprintDetectSpeed;
		}
	}
	else
	{
		bIsSprinting = false;
	}

	if (bIsSprinting)
	{
		// 배고픔·갈증 낮을수록 빨리 닳음 (최대 2배)
		const float EffDrain = StaminaDrainRate * (2.f - VitalFactor);
		CurrentStamina = FMath::Max(0.f, CurrentStamina - EffDrain * DeltaTime);

		if (CurrentStamina <= 0.f)
		{
			bSprintLocked   = true;
			SprintLockTimer = SprintLockDuration;
			bIsSprinting    = false;
		}
	}
	else
	{
		// 회복 속도 → 활력 비례 (최소 10%)
		const float EffRegen = StaminaRegenRate * FMath::Max(0.1f, VitalFactor);
		CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + EffRegen * DeltaTime);
	}

	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);

	// CanSprint 상태 변화 → BP 알림
	const bool bCur = CanSprint();
	if (bCur != bLastCanSprint)
	{
		bLastCanSprint = bCur;
		OnSprintStateChanged.Broadcast(bCur);
	}
}

void UDeliveryComponent::EatFood(float Nutrition)
{
	Hunger = FMath::Min(HungerMax, Hunger + Nutrition);
}

void UDeliveryComponent::DrinkWater(float Hydration)
{
	Thirst = FMath::Min(ThirstMax, Thirst + Hydration);
}

// ── 무게 페널티 (인벤 참조) ───────────────────────────────────────────

float UDeliveryComponent::GetWeightSpeedPenalty() const
{
	if (AActor* OwnerActor = GetOwner())
	{
		if (UBagInventoryComponent* Bag = OwnerActor->FindComponentByClass<UBagInventoryComponent>())
		{
			// 무게 비율 0~1 → 속도 1.0 ~ 0.5
			const float Ratio = Bag->GetWeightRatio();
			return FMath::Lerp(1.f, 0.5f, Ratio);
		}
	}
	return 1.f;
}

// ── 배달 ──────────────────────────────────────────────────────────────

void UDeliveryComponent::Interact()
{
	TryDeliverPackage();
	if (CarriedPackages.Num() < MaxCarryOrders)
		TryPickupPackage();
}

void UDeliveryComponent::TryPickupPackage()
{
	for (auto& Weak : NearbyPackages)
	{
		APackageActor* Package = Weak.Get();
		if (Package && Package->bIsAvailable)
		{
			Package->PickupPackage(this);
			CarriedPackages.Add(Package);
			OnOrderCountChanged.Broadcast(CarriedPackages.Num());
			return;
		}
	}
}

void UDeliveryComponent::TryDeliverPackage()
{
	for (auto& WeakDropoff : NearbyDropoffs)
	{
		ADeliveryPointActor* Dropoff = WeakDropoff.Get();
		if (!Dropoff || !Dropoff->bIsActive) continue;

		for (int32 i = CarriedPackages.Num() - 1; i >= 0; --i)
		{
			APackageActor* Package = CarriedPackages[i];
			if (!Package || Package->Order.OrderID != Dropoff->BoundOrderID) continue;

			AddMoney(Package->Order.Reward);
			Dropoff->OnOrderDelivered.Broadcast(GetOwner(), Package->Order.Reward);
			Dropoff->DeactivateDropoff();
			CarriedPackages.RemoveAt(i);
			OnOrderCountChanged.Broadcast(CarriedPackages.Num());

			// GetWorld() nullptr 체크 — 엣지 케이스 크래시 방지
			if (!GetWorld()) return;
			if (AGamesetGameMode* GM = Cast<AGamesetGameMode>(GetWorld()->GetAuthGameMode()))
				if (GM->DeliveryManager)
					GM->DeliveryManager->NotifyOrderDelivered(Package->Order.OrderID);
			return;
		}
	}
}

// ── 돈 ────────────────────────────────────────────────────────────────

void UDeliveryComponent::AddMoney(float Amount)
{
	Money += Amount;
	OnMoneyChanged.Broadcast(Money, Amount);
}

bool UDeliveryComponent::SpendMoney(float Amount)
{
	if (Money < Amount) return false;
	Money -= Amount;
	OnMoneyChanged.Broadcast(Money, -Amount);
	return true;
}

// ── 오버랩 / 업그레이드 ───────────────────────────────────────────────

void UDeliveryComponent::AddNearbyPackage   (APackageActor*       P) { NearbyPackages.AddUnique(P); }
void UDeliveryComponent::RemoveNearbyPackage(APackageActor*       P) { NearbyPackages.Remove(P);    }
void UDeliveryComponent::AddNearbyDropoff   (ADeliveryPointActor* D) { NearbyDropoffs.AddUnique(D); }
void UDeliveryComponent::RemoveNearbyDropoff(ADeliveryPointActor* D) { NearbyDropoffs.Remove(D);    }

void UDeliveryComponent::ApplyStaminaUpgrade(float Multiplier)
{
	MaxStamina     = BaseMaxStamina * Multiplier;
	CurrentStamina = FMath::Min(CurrentStamina, MaxStamina);
}
