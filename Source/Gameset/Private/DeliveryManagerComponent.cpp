#include "DeliveryManagerComponent.h"
#include "PackageActor.h"
#include "DeliveryPointActor.h"
#include "UpgradeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

UDeliveryManagerComponent::UDeliveryManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	CustomerNames =
	{
		FText::FromString(TEXT("김민준")), FText::FromString(TEXT("이수연")),
		FText::FromString(TEXT("박지호")), FText::FromString(TEXT("최은혜")),
		FText::FromString(TEXT("정현우")), FText::FromString(TEXT("한지연")),
		FText::FromString(TEXT("윤성민")), FText::FromString(TEXT("임대영")),
		FText::FromString(TEXT("오세진")), FText::FromString(TEXT("강미래")),
	};
}

void UDeliveryManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ★ 진단 로그 — 이 메시지가 Output Log에 보이면 BeginPlay 실행 중
	UE_LOG(LogTemp, Warning, TEXT("★★★ DeliveryManagerComponent BeginPlay 실행됨 ★★★"));

	// 레벨에 배치된 픽업/드롭오프 액터 자동 탐색
	if (PickupPoints.IsEmpty())
	{
		TArray<AActor*> Found;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APackageActor::StaticClass(), Found);
		for (AActor* A : Found)
			if (APackageActor* P = Cast<APackageActor>(A))
				PickupPoints.Add(P);
	}

	if (DropoffPoints.IsEmpty())
	{
		TArray<AActor*> Found;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADeliveryPointActor::StaticClass(), Found);
		for (AActor* A : Found)
			if (ADeliveryPointActor* D = Cast<ADeliveryPointActor>(A))
				DropoffPoints.Add(D);
	}

	// InitialSpawnDelay(기본 2초) 후 첫 오더 생성
	UE_LOG(LogTemp, Warning, TEXT("★★★ 배달 타이머 시작: %.1f초 후 첫 주문 생성 ★★★"), InitialSpawnDelay);
	GetWorld()->GetTimerManager().SetTimer(
		InitSpawnTimerHandle,
		this, &UDeliveryManagerComponent::TrySpawnNewOrder,
		InitialSpawnDelay,
		false  // 1회만
	);
}

void UDeliveryManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateTimers(DeltaTime);

	SpawnTimer += DeltaTime;
	// Pending + Active 합산이 한도를 넘지 않을 때만 신규 오더 생성
	if (SpawnTimer >= OrderSpawnInterval
		&& PendingOrders.Num() < MaxPendingOrders
		&& ActiveOrders.Num() < MaxActiveOrders)
	{
		TrySpawnNewOrder();
		SpawnTimer = 0.f;
	}
}

void UDeliveryManagerComponent::TrySpawnNewOrder()
{
	UE_LOG(LogTemp, Warning, TEXT("★★★ TrySpawnNewOrder 호출됨! Pending=%d MaxPending=%d ★★★"),
		PendingOrders.Num(), MaxPendingOrders);

	// Pending 큐가 가득 차 있으면 새 오더 생성 안 함
	if (PendingOrders.Num() >= MaxPendingOrders) return;

	// ── 랜덤 음식점 / 물품 이름 풀 ────────────────────────────────────
	static const TArray<FString> RestaurantNames = {
		TEXT("맛있닭 강남점"),   TEXT("피자헛 서초점"),   TEXT("버거킹 홍대점"),
		TEXT("스시로 잠실점"),   TEXT("BBQ치킨 신촌점"),  TEXT("교촌치킨 마포점"),
		TEXT("도미노피자 건대점"), TEXT("롯데리아 종로점"), TEXT("마마스치킨 이태원점"),
		TEXT("본죽 여의도점"),
	};
	static const TArray<FString> ItemDescriptions = {
		TEXT("치킨 2마리 + 콜라 1.5L"),        TEXT("피자 라지 1판 + 사이드 2개"),
		TEXT("버거세트 2개 + 감자튀김 추가"),   TEXT("초밥 모둠 20피스"),
		TEXT("치킨버거 3개 + 음료"),             TEXT("순살치킨 1마리 + 치킨무"),
		TEXT("불고기피자 + 애플사이다"),         TEXT("새우버거 세트 + 아이스크림"),
		TEXT("참치김밥 + 된장찌개"),             TEXT("전복죽 2인분 + 반찬 세트"),
	};
	static const TArray<FString> DropoffAreaNames = {
		TEXT("역삼 래미안"), TEXT("신촌 현대아파트"), TEXT("잠실 엘스"), TEXT("마포 공덕 자이"),
		TEXT("홍대 오피스텔"), TEXT("이태원 빌라"), TEXT("여의도 파크원"), TEXT("건대 스타시티"),
	};

	// ── 사용 중인 포인트 제외 (Active + Pending 모두) ────────────────
	TSet<AActor*> UsedPickups;
	TSet<AActor*> UsedDropoffs;
	for (const FDeliveryOrder& O : ActiveOrders)
	{
		if (O.PickupActor)  UsedPickups.Add(O.PickupActor);
		if (O.DropoffActor) UsedDropoffs.Add(O.DropoffActor);
	}
	for (const FDeliveryOrder& O : PendingOrders)
	{
		if (O.PickupActor)  UsedPickups.Add(O.PickupActor);
		if (O.DropoffActor) UsedDropoffs.Add(O.DropoffActor);
	}

	TArray<APackageActor*>       FreePickups;
	TArray<ADeliveryPointActor*> FreeDropoffs;

	for (APackageActor* P : PickupPoints)
		if (P && !UsedPickups.Contains(P))
			FreePickups.Add(P);

	for (ADeliveryPointActor* D : DropoffPoints)
		if (D && !D->bIsActive && !UsedDropoffs.Contains(D))
			FreeDropoffs.Add(D);

	// 레벨에 배달 액터가 없어도 가상 주문(알림 전용)을 생성한다.
	// PickupActor / DropoffActor 가 nullptr인 주문은 AcceptOrder에서 안전하게 처리된다.
	const bool bHasActors = !FreePickups.IsEmpty() && !FreeDropoffs.IsEmpty();

	APackageActor*       Pickup  = bHasActors ? FreePickups [FMath::RandRange(0, FreePickups.Num()  - 1)] : nullptr;
	ADeliveryPointActor* Dropoff = bHasActors ? FreeDropoffs[FMath::RandRange(0, FreeDropoffs.Num() - 1)] : nullptr;

	if (!bHasActors)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[배달] 레벨에 PackageActor/DeliveryPointActor 없음 → 가상 주문 생성 (알림 테스트용)"));
	}

	// TimeBonus 업그레이드: UpgradeComponent에서 추가 시간 가산
	float TimeBonusSec = 0.f;
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		if (UUpgradeComponent* UpgradeComp = PlayerPawn->FindComponentByClass<UUpgradeComponent>())
		{
			TimeBonusSec = UpgradeComp->GetTimeBonusSec();
		}
	}
	const float FinalTimeLimit = BaseTimeLimit + TimeBonusSec;

	// ── 픽업→드롭오프 실제 거리(m) 계산 (UU → m: ÷100) ──────────────
	// 액터가 없는 가상 주문은 랜덤 거리(500~3000m)를 사용한다.
	const float DistanceM = (Pickup && Dropoff)
		? FVector::Dist(Pickup->GetActorLocation(), Dropoff->GetActorLocation()) / 100.f
		: FMath::RandRange(500.f, 3000.f);

	// ── 랜덤 배달 정보 선택 ──────────────────────────────────────────
	const FString PickupNameStr  = RestaurantNames [FMath::RandRange(0, RestaurantNames.Num()  - 1)];
	const FString ItemDescStr    = ItemDescriptions[FMath::RandRange(0, ItemDescriptions.Num() - 1)];
	const FString DropoffAreaStr = DropoffAreaNames [FMath::RandRange(0, DropoffAreaNames.Num() - 1)];
	// 드롭오프 이름에 고객 이름 포함
	const FText   CustomerName   = CustomerNames[FMath::RandRange(0, CustomerNames.Num() - 1)];
	const FString DropoffNameStr = FString::Printf(TEXT("%s (%s 고객)"), *DropoffAreaStr, *CustomerName.ToString());

	// ── 오더 생성 (Pending 상태) ──────────────────────────────────────
	FDeliveryOrder NewOrder;
	NewOrder.OrderID              = GenerateOrderID();
	NewOrder.CustomerName         = CustomerName;
	NewOrder.TimeLimit            = FinalTimeLimit;
	NewOrder.TimeRemaining        = FinalTimeLimit;
	NewOrder.Reward               = (Pickup && Dropoff) ? CalcReward(Pickup, Dropoff) : BaseReward;
	NewOrder.Status               = EOrderStatus::Pending;   // 수락 대기 상태로 시작
	NewOrder.PickupActor          = Pickup;
	NewOrder.DropoffActor         = Dropoff;
	NewOrder.DistanceMeters       = DistanceM;
	NewOrder.ItemDescription      = FText::FromString(ItemDescStr);
	NewOrder.PickupName           = FText::FromString(PickupNameStr);
	NewOrder.DropoffName          = FText::FromString(DropoffNameStr);
	NewOrder.PendingExpireTime    = 30.f;
	NewOrder.PendingTimeRemaining = 30.f;

	// Pending 큐에 추가 — 아직 PackageActor/DeliveryPointActor는 활성화하지 않음
	PendingOrders.Add(NewOrder);

	// 폰 앱 UI에 알림 브로드캐스트
	UE_LOG(LogTemp, Warning, TEXT("★★★ OnNewOrderPending 브로드캐스트! OrderID=%s, 리스너수=%d ★★★"),
		*NewOrder.OrderID, OnNewOrderPending.IsBound() ? 1 : 0);
	OnNewOrderPending.Broadcast(NewOrder);
}

void UDeliveryManagerComponent::UpdateTimers(float DeltaTime)
{
	// ── Pending 오더 타이머 처리 ─────────────────────────────────────
	// 수락 안 하면 PendingExpireTime(기본 30초) 후 자동 거절
	for (int32 i = PendingOrders.Num() - 1; i >= 0; --i)
	{
		FDeliveryOrder& Order = PendingOrders[i];
		Order.PendingTimeRemaining -= DeltaTime;

		if (Order.PendingTimeRemaining <= 0.f)
		{
			// 시간 초과 → 자동 거절 처리
			const FString ExpiredID = Order.OrderID;
			PendingOrders.RemoveAt(i);
			// RejectOrder 내부 로직을 직접 호출 (이미 배열에서 제거했으므로 ID 기반 탐색만 수행)
			// PackageActor/DeliveryPointActor는 아직 활성화되지 않았으므로 풀 복원만 필요 없음
			OnOrderExpired.Broadcast(ExpiredID);
		}
	}

	// ── Active 오더 타이머 처리 ──────────────────────────────────────
	for (int32 i = ActiveOrders.Num() - 1; i >= 0; --i)
	{
		FDeliveryOrder& Order = ActiveOrders[i];
		if (Order.Status == EOrderStatus::Delivered || Order.Status == EOrderStatus::Failed) continue;

		Order.TimeRemaining -= DeltaTime;
		if (Order.TimeRemaining > 0.f) continue;

		// 시간 초과 처리
		Order.Status = EOrderStatus::Failed;
		TotalFailed++;

		if (APackageActor* P = Cast<APackageActor>(Order.PickupActor))
		{
			P->bIsAvailable = false;
			P->SetActorHiddenInGame(true);
			P->SetActorEnableCollision(false);

			// 만료된 PackageActor를 FreePickups 풀(PickupPoints)에 복원
			// — 복원하지 않으면 게임이 길어질수록 픽업 포인트가 고갈됨
			if (!PickupPoints.Contains(P))
			{
				PickupPoints.Add(P);
			}
		}
		if (ADeliveryPointActor* D = Cast<ADeliveryPointActor>(Order.DropoffActor))
		{
			D->DeactivateDropoff();

			// 만료된 DeliveryPointActor를 FreeDropoffs 풀(DropoffPoints)에 복원
			if (!DropoffPoints.Contains(D))
			{
				DropoffPoints.Add(D);
			}
		}

		OnOrderExpired.Broadcast(Order.OrderID);
		ActiveOrders.RemoveAt(i);
	}
}

bool UDeliveryManagerComponent::AcceptOrder(const FString& OrderID)
{
	// PendingOrders에서 해당 OrderID 탐색
	for (int32 i = 0; i < PendingOrders.Num(); ++i)
	{
		if (PendingOrders[i].OrderID != OrderID) continue;

		// Active 오더가 이미 최대치면 수락 불가
		if (ActiveOrders.Num() >= MaxActiveOrders)
		{
			UE_LOG(LogTemp, Warning, TEXT("[배달] AcceptOrder 실패: ActiveOrders 최대치 (%d)"), MaxActiveOrders);
			return false;
		}

		// 오더 상태를 Available(진행 중)로 변경
		FDeliveryOrder Order = PendingOrders[i];
		Order.Status = EOrderStatus::Available;

		// PackageActor / DeliveryPointActor 활성화 (가상 주문이면 nullptr이므로 null 체크)
		if (APackageActor* P = Cast<APackageActor>(Order.PickupActor))
		{
			P->ActivateOrder(Order);
		}
		else if (!Order.PickupActor)
		{
			UE_LOG(LogTemp, Log, TEXT("[배달] 가상 주문 수락: PickupActor 없음 (레벨에 PackageActor 미배치)"));
		}
		if (ADeliveryPointActor* D = Cast<ADeliveryPointActor>(Order.DropoffActor))
		{
			D->ActivateDropoff(Order.OrderID);
		}

		// Pending 큐에서 제거 → Active 큐에 추가
		PendingOrders.RemoveAt(i);
		ActiveOrders.Add(Order);

		// 기존 OnNewOrderCreated 델리게이트 재사용 — UI가 오더 목록을 갱신하도록 알림
		OnNewOrderCreated.Broadcast(Order);

		UE_LOG(LogTemp, Log, TEXT("[배달] 오더 수락: %s (%s → %s)"),
			*Order.OrderID, *Order.PickupName.ToString(), *Order.DropoffName.ToString());
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("[배달] AcceptOrder 실패: OrderID '%s' 를 PendingOrders에서 찾을 수 없음"), *OrderID);
	return false;
}

bool UDeliveryManagerComponent::RejectOrder(const FString& OrderID)
{
	// PendingOrders에서 해당 OrderID 탐색
	for (int32 i = 0; i < PendingOrders.Num(); ++i)
	{
		if (PendingOrders[i].OrderID != OrderID) continue;

		FDeliveryOrder Order = PendingOrders[i];
		Order.Status = EOrderStatus::Rejected;

		// PackageActor/DeliveryPointActor는 Pending 중에는 활성화되지 않았으므로
		// 풀(PickupPoints / DropoffPoints)에 복원만 보장하면 됨.
		// (TrySpawnNewOrder에서 UsedPickups/UsedDropoffs 제외 로직이 Pending 참조를 포함하므로
		//  RemoveAt 후 자동으로 풀로 복귀됨 — 별도 추가 불필요)

		PendingOrders.RemoveAt(i);

		// 거절된 오더도 만료 이벤트로 알림 (UI 정리용)
		OnOrderExpired.Broadcast(Order.OrderID);

		UE_LOG(LogTemp, Log, TEXT("[배달] 오더 거절: %s"), *Order.OrderID);
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("[배달] RejectOrder 실패: OrderID '%s' 를 PendingOrders에서 찾을 수 없음"), *OrderID);
	return false;
}

void UDeliveryManagerComponent::NotifyOrderDelivered(const FString& OrderID, APawn* DeliveryPawn)
{
	for (int32 i = ActiveOrders.Num() - 1; i >= 0; --i)
	{
		if (ActiveOrders[i].OrderID == OrderID)
		{
			TotalDelivered++;

			// TipMultiplier 업그레이드 적용:
			// 배달 완료 시점에 실제 배달한 플레이어 Pawn의 UpgradeComponent에서 멀티플라이어를 가져와 보상에 반영.
			// DeliveryPawn이 없으면 World의 플레이어 0번 Pawn을 폴백으로 사용.
			APawn* PawnToCheck = DeliveryPawn;
			if (!PawnToCheck)
			{
				PawnToCheck = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
			}

			if (PawnToCheck)
			{
				if (UUpgradeComponent* UpgradeComp = PawnToCheck->FindComponentByClass<UUpgradeComponent>())
				{
					const float Multiplier = UpgradeComp->GetRewardMultiplier();
					// 보상에 멀티플라이어 적용 (반올림하여 정수 단위로 지급)
					ActiveOrders[i].Reward = FMath::RoundToFloat(ActiveOrders[i].Reward * Multiplier);
				}
			}

			ActiveOrders.RemoveAt(i);
			return;
		}
	}
}

FString UDeliveryManagerComponent::GenerateOrderID()
{
	return FString::Printf(TEXT("ORD-%04d"), ++OrderCounter);
}

float UDeliveryManagerComponent::CalcReward(APackageActor* Pickup, ADeliveryPointActor* Dropoff, APawn* DeliveryPawn) const
{
	// 픽업~드롭오프 거리 기반 기본 보상 계산 (1000 UU 당 50원 추가)
	float Dist  = FVector::Dist(Pickup->GetActorLocation(), Dropoff->GetActorLocation());
	float Bonus = Dist / 1000.f * 50.f;
	float Base  = FMath::RoundToFloat(BaseReward + Bonus);

	// TipMultiplier 업그레이드 적용:
	// DeliveryPawn이 전달된 경우(배달 완료 시점)에만 멀티플라이어를 곱함.
	// 오더 생성 시점(TrySpawnNewOrder)에는 DeliveryPawn이 nullptr이므로 기본값 사용.
	if (DeliveryPawn)
	{
		if (UUpgradeComponent* UpgradeComp = DeliveryPawn->FindComponentByClass<UUpgradeComponent>())
		{
			Base = FMath::RoundToFloat(Base * UpgradeComp->GetRewardMultiplier());
		}
	}

	return Base;
}
